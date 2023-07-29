#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <sys/time.h>
#include "esp_sntp.h"

#include "main.h"
#include "ui_task.h"
#include "wifi.h"
#include "online_requests.h"
#include "clock.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
// max time to put new command to timer's queue
#define XTIMER_QUEUE_DELAY_MS	100

// event bits
#define BASIC_DATA_NEEDS_UPDATE_BIT	(1 << 0)
#define BASIC_DATA_IS_UPDATED_BIT	(1 << 1)

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

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const char *default_timezone = "GMT0";

static struct clock_data main_clock;
static TaskHandle_t clock_task_handle;

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

	// create 1 minute timer
	main_clock.one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(60000), pdFALSE, NULL, one_minute_timer_callback);
	assert(main_clock.one_minute_timer_handle);

	clock_task_handle = xTaskGetCurrentTaskHandle();
	assert(clock_task_handle);

	main_clock.status = clock_not_set;

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, CLOCK_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// stop timer to set it with new values
		xTimerStop(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

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

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

void Clock_Update(int TimeUnix, const char *TimezoneString){

	struct timeval tv_rcvd, tv_now;

	// put recieved unix time to the structure
	tv_rcvd.tv_sec = TimeUnix;
	tv_rcvd.tv_usec = 0;

	// put actual unix time to the structure
	time(&tv_now.tv_sec);
	tv_now.tv_usec = 0;

	// if adjusting time not possible
	if(0 != adjtime(&tv_rcvd, &tv_now)){
		// set time directly
		settimeofday(&tv_rcvd, NULL);
	}

	if(main_clock.current_timezone != TimezoneString){

		main_clock.current_timezone = TimezoneString;
		if(NULL != TimezoneString){

			// set recieved timezone
			setenv("TZ", main_clock.current_timezone, 1);
			tzset();
		}
	}

	main_clock.status = clock_sync;

	xTaskNotifyGive(clock_task_handle);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* function called when clock has not_set state */
static void clock_not_set_routine(void){

	if(main_clock.timeinfo.tm_year <= (int)(2020U - 1900U)) {

		OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);
		xTimerChangePeriod(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(20000), pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
		xTimerStart(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	}
	else {

		main_clock.status = clock_not_sync;
		xTaskNotifyGive(clock_task_handle);
	}
}

/* function called when clock has not_sync state */
static void clock_not_sync_routine(void){

	OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);

	// calculate remaining seconds to call timer right after new minute value
	main_clock.timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(main_clock.timeinfo.tm_sec);

	// set the timer with calculated value and start it
	xTimerChangePeriod(main_clock.one_minute_timer_handle, main_clock.timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
}

static void clock_sync_routine(void){

	if(BASIC_DATA_RESFRESH_MINUTE == main_clock.timeinfo.tm_min){

		OnlineRequest_Send(ONLINEREQ_BASIC_UPDATE, NULL);
		main_clock.status = clock_sync_pending;
		xTimerChangePeriod(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(5000), pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
		xTimerStart(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	}
	else{

		// calculate remaining seconds to call timer right after new minute value
		main_clock.timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(main_clock.timeinfo.tm_sec);

		// set the timer with calculated value and start it
		xTimerChangePeriod(main_clock.one_minute_timer_handle, main_clock.timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
		xTimerStart(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	}

	UI_ReportEvt(UI_EVT_TIME_CHANGED, &main_clock.timeinfo);
}

static void clock_sync_pending_routine(void){

	main_clock.status = clock_not_sync;

	// calculate remaining seconds to call timer right after new minute value
	main_clock.timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(main_clock.timeinfo.tm_sec);

	// set the timer with calculated value and start it
	xTimerChangePeriod(main_clock.one_minute_timer_handle, main_clock.timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(main_clock.one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
}

/* notify clock_task to perform clock routine */
static void one_minute_timer_callback(TimerHandle_t xTimer){

	xTaskNotifyGive(clock_task_handle);
}
