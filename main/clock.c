#include "main.h"
#include <sys/time.h>

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
// the time when update request is sent
#define BASIC_DATA_RESFRESH_MINUTE		9

// max time to put new command to timer's queue
#define XTIMER_QUEUE_DELAY_MS			100

// event bits
#define TIME_SYNC_BIT					(1 << 1)
#define TIMEZONE_SYNC_BIT				(1 << 2)
#define BASIC_DATA_RESFRESH_BIT			(1 << 3)

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

// task general
typedef enum {

	clock_refresh = 0,
	clock_time_update,

} clock_routine_type_t;

typedef void (*clock_routine)(void *arg);

struct clock_queue_data {

	clock_routine_type_t type;
	void *arg;
};

// clock
enum {clock_not_set = 0, clock_not_sync, clock_sync};

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
static void clock_routine_request(clock_routine_type_t type, void *arg);

static void clock_refresh_routine(void *arg);
static void clock_time_update_routine(void *arg);

static void clock_not_set_job(void);
static void clock_not_sync_job(void);
static void clock_sync_job(void);

static void one_minute_timer_callback(TimerHandle_t xTimer);
static void next_minute_timer_start(struct clock_data *clock);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const char *default_timezone = "GMT0";

static struct clock_data main_clock;
static QueueHandle_t clock_queue_handle;
static EventGroupHandle_t clock_bits_handle;

static const clock_routine clock_routines_tab[] = {

		[clock_refresh] = clock_refresh_routine,
		[clock_time_update] = clock_time_update_routine,
};

/******************************************************************************************************************
 *
 * Clock task
 *
 ******************************************************************************************************************/
void Clock_Task(void *arg){

	BaseType_t ret;
	struct clock_queue_data data;

	// set default timezone if current is not set
	if(NULL == main_clock.current_timezone){

		setenv("TZ", default_timezone, 1);
		tzset();
	}

	// init needed resources
	main_clock.one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(60000), pdFALSE, NULL, one_minute_timer_callback);
	assert(main_clock.one_minute_timer_handle);

	clock_queue_handle = xQueueCreate(2U, sizeof(struct clock_queue_data));
	assert(clock_queue_handle);

	clock_bits_handle = xEventGroupCreate();
	assert(clock_bits_handle);

	main_clock.status = clock_not_set;

	Alarm_InitResources();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, CLOCK_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	clock_routine_request(clock_refresh, NULL);

	while(1){

		ret = xQueueReceive(clock_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			clock_routines_tab[data.type](data.arg);
		}
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* inform the task that timezone has been updated */
void Timezone_Update(const char *TimezoneString){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	// skip if rcvd timezone is the same
	if(main_clock.current_timezone != TimezoneString){

		main_clock.current_timezone = TimezoneString;
		if(NULL != TimezoneString){

			// set recieved timezone
			setenv("TZ", main_clock.current_timezone, 1);
			tzset();
		}
	}

	// if timezone was not synchronised
	if(!(bits & TIMEZONE_SYNC_BIT)){

		xEventGroupSetBits(clock_bits_handle, TIMEZONE_SYNC_BIT);

		clock_routine_request(clock_refresh, NULL);
	}
}

/* inform the task that time has been updated */
void TimeUpdated(void){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	if(!(bits & TIME_SYNC_BIT)){

		xEventGroupSetBits(clock_bits_handle, TIME_SYNC_BIT);

		clock_routine_request(clock_refresh, NULL);
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* send routine to be performed */
static void clock_routine_request(clock_routine_type_t type, void *arg){

	struct clock_queue_data data;

	data.type = type;
	data.arg = arg;

	xQueueSend(clock_queue_handle, &data, pdMS_TO_TICKS(100));
}

/* refresh the clock state and value */
static void clock_refresh_routine(void *arg){

	xTimerStop(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	// get current system time
	time(&main_clock.now);
	localtime_r(&main_clock.now, &main_clock.timeinfo);

	switch(main_clock.status){

	case clock_not_set:
		 clock_not_set_job();
		break;

	case clock_not_sync:
		 clock_not_sync_job();
		break;

	case clock_sync:
		 clock_sync_job();
		break;

	default:
		break;
	}
}

static void clock_time_update_routine(void *arg){


}

/* function called when clock has not_set state */
static void clock_not_set_job(void){

	// if time is below 2020 it means is not set
	if(main_clock.timeinfo.tm_year <= (int)(2020U - 1900U)){

		// send update request
		OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
		UI_ReportEvt(UI_EVT_CLOCK_NOT_SYNC, 0);
		next_minute_timer_start(&main_clock);
	}
	else {

		main_clock.status = clock_not_sync;
		clock_routine_request(clock_refresh, NULL);
	}
}

/* function called when clock has not_sync state */
static void clock_not_sync_job(void){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	if((bits & TIMEZONE_SYNC_BIT) && (bits & TIME_SYNC_BIT)){

		main_clock.status = clock_sync;
		clock_routine_request(clock_refresh, NULL);
	}
	else{

		if(!(bits & TIMEZONE_SYNC_BIT)){

			// if timezone is not sync
			OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);
		}

		if(!(bits & TIME_SYNC_BIT)){

			// if time is not sync
			OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
		}

		next_minute_timer_start(&main_clock);
		UI_ReportEvt(UI_EVT_CLOCK_NOT_SYNC, 0);
	}

	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
}

/* function called when clock has sync state */
static void clock_sync_job(void){

	EventBits_t bits = xEventGroupGetBits(clock_bits_handle);

	// if the time for sync has come and were not refreshed this time yet
	// BASIC_DATA_RESFRESH_BIT is used to perform synchornisation only once during
	// the time, the function clock_sync_job can be called multiple times in one minute
	if((BASIC_DATA_RESFRESH_MINUTE == main_clock.timeinfo.tm_min) && (bits & BASIC_DATA_RESFRESH_BIT)){

		xEventGroupClearBits(clock_bits_handle, (TIME_SYNC_BIT | TIMEZONE_SYNC_BIT | BASIC_DATA_RESFRESH_BIT));

		OnlineRequest_Send(ONLINEREQ_TIME_UPDATE, NULL);
		OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);

		main_clock.status = clock_not_sync;
	}
	else{

		xEventGroupSetBits(clock_bits_handle, BASIC_DATA_RESFRESH_BIT);
		UI_ReportEvt(UI_EVT_CLOCK_SYNC, 0);
	}

	next_minute_timer_start(&main_clock);
	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
}

/* notify clock_task to perform clock routine */
static void one_minute_timer_callback(TimerHandle_t xTimer){

	clock_routine_request(clock_refresh, NULL);
}

/* prepare and start the timer when minute change */
static void next_minute_timer_start(struct clock_data *clock){

	// calculate remaining seconds to call timer right after new minute value
	clock->timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(clock->timeinfo.tm_sec);

	xTimerChangePeriod(clock->one_minute_timer_handle, clock->timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(clock->one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
}
