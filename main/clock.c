#include "main.h"
#include <sys/time.h>
//#include "esp_sntp.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
// max time to put new command to timer's queue
#define XTIMER_QUEUE_DELAY_MS	100

// event bits
#define TIME_SET_BIT			(1 << 0)
#define TIME_SYNC_BIT			(1 << 1)
#define TIMEZONE_SYNC_BIT		(1 << 2)
#define BASIC_DATA_RESFRESH_BIT	(1 << 3)

/**************************************************************
 *
 *	Macros
 *
 ***************************************************************/
#define TICKS_TO_NEXT_MINUTE(x) (pdMS_TO_TICKS(((int)60 - x) * 1000U))

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
enum {clock_not_set = 0, clock_not_sync, clock_sync_pending, clock_sync};

struct clock_data {

	time_t now;
	struct tm timeinfo;
	TickType_t timer_next_ticks_value;
	TimerHandle_t one_minute_timer_handle;
	const char *current_timezone;
	uint8_t status;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void clock_not_set_routine(void);
static void clock_not_sync_routine(void);
static void clock_sync_routine(void);
static void clock_sync_pending_routine(void);

static void one_minute_timer_callback(TimerHandle_t xTimer);
static void next_minute_timer_start(struct clock_data *clock);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const char *default_timezone = "GMT0";

static struct clock_data main_clock;//TODO mutex!
static TaskHandle_t clock_task_handle;
static EventGroupHandle_t clock_bits_handle;

/******************************************************************************************************************
 *
 * Clock task
 *
 ******************************************************************************************************************/
void Clock_Task(void *arg){

	// set default timezone if current is not set
	if(NULL == main_clock.current_timezone){

		setenv("TZ", default_timezone, 1);
		tzset();
	}

	// init needed resources
	main_clock.one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(60000), pdFALSE, NULL, one_minute_timer_callback);
	assert(main_clock.one_minute_timer_handle);

	clock_task_handle = xTaskGetCurrentTaskHandle();
	assert(clock_task_handle);

	clock_bits_handle = xEventGroupCreate();
	assert(clock_bits_handle);

	main_clock.status = clock_not_set;

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, CLOCK_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// get current system time
		time(&main_clock.now);
		localtime_r(&main_clock.now, &main_clock.timeinfo);

		switch(main_clock.status){

		case clock_not_set:
			 clock_not_set_routine();
			break;

		case clock_not_sync:
			 clock_not_sync_routine();
			break;

		case clock_sync_pending:
			 clock_sync_pending_routine();
			break;

		case clock_sync:
			 clock_sync_routine();
			break;

		default:
			break;
		}

		// wait for next event
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		xTimerStop(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void Timezone_Update(const char *TimezoneString){

	// skip if rcvd timezone is the same
	if(main_clock.current_timezone != TimezoneString){

		main_clock.current_timezone = TimezoneString;
		if(NULL != TimezoneString){

			// set recieved timezone
			setenv("TZ", main_clock.current_timezone, 1);
			tzset();
		}
	}

	xEventGroupSetBits(clock_bits_handle, TIMEZONE_SYNC_BIT);
}

void TimeUpdated(void){

	xEventGroupSetBits(clock_bits_handle, TIME_SYNC_BIT);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* function called when clock has not_set state */
static void clock_not_set_routine(void){

	EventBits_t bits = 0;

	// if time is below 2020 it means is not set
	if(main_clock.timeinfo.tm_year <= (int)(2020U - 1900U)){

		// send update request and wait
		OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
		bits = xEventGroupWaitBits(clock_bits_handle, TIME_SYNC_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(TIMEOUT_MS));
		if(bits & TIME_SYNC_BIT){

			xEventGroupSetBits(clock_bits_handle, TIME_SET_BIT);
			main_clock.status = clock_not_sync;
			UI_ReportEvt(UI_EVT_CLOCK_NOT_SYNC, 0);
		}

	}
	else {

		xEventGroupSetBits(clock_bits_handle, TIME_SET_BIT);
		main_clock.status = clock_not_sync;
		UI_ReportEvt(UI_EVT_CLOCK_NOT_SYNC, 0);
	}

	xTaskNotifyGive(clock_task_handle);
}

/* function called when clock has not_sync state */
static void clock_not_sync_routine(void){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	if(!(bits & TIMEZONE_SYNC_BIT)){

		// if timezone is not sync
		OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);
	}

	if(!(bits & TIME_SYNC_BIT)){

		// if time is not sync
		OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
	}

	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
	main_clock.status = clock_sync_pending;
	xTaskNotifyGive(clock_task_handle);
}

/* function called when clock has sync_pending state */
static void clock_sync_pending_routine(void){

	EventBits_t bits = xEventGroupWaitBits(clock_bits_handle, (TIME_SYNC_BIT | TIMEZONE_SYNC_BIT), pdFALSE,
			pdTRUE, pdMS_TO_TICKS(TIMEOUT_MS));
	if((bits & TIME_SYNC_BIT) && (bits & TIMEZONE_SYNC_BIT)){

		// if time and timezone has been sync
		main_clock.status = clock_sync;
		UI_ReportEvt(UI_EVT_CLOCK_SYNC, 0);
		xTaskNotifyGive(clock_task_handle);

	}
	else{

		// wait till next minute if no
		main_clock.status = clock_not_sync;
		UI_ReportEvt(UI_EVT_CLOCK_NOT_SYNC, 0);
		next_minute_timer_start(&main_clock);
	}
}

/* function called when clock has sync state */
static void clock_sync_routine(void){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	// if the time for sync has come and were not refreshed this time yet
	// BASIC_DATA_RESFRESH_BIT is used to perform synchornisation only once during
	// the time, the function clock_sync_routine can be called multiple times in one minute
	if((BASIC_DATA_RESFRESH_MINUTE == main_clock.timeinfo.tm_min) && (bits & BASIC_DATA_RESFRESH_BIT)){

		xEventGroupClearBits(clock_bits_handle, (TIME_SYNC_BIT | TIMEZONE_SYNC_BIT | BASIC_DATA_RESFRESH_BIT));

		OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
		OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);

		main_clock.status = clock_sync_pending;
		xTaskNotifyGive(clock_task_handle);
	}
	else{

		xEventGroupSetBits(clock_bits_handle, BASIC_DATA_RESFRESH_BIT);
		next_minute_timer_start(&main_clock);
	}

	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
}

/* notify clock_task to perform clock routine */
static void one_minute_timer_callback(TimerHandle_t xTimer){

	xTaskNotifyGive(clock_task_handle);
}

/* prepare and start the timer when minute change */
static void next_minute_timer_start(struct clock_data *clock){

	// calculate remaining seconds to call timer right after new minute value
	clock->timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(clock->timeinfo.tm_sec);

	xTimerChangePeriod(clock->one_minute_timer_handle, clock->timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(clock->one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
}
