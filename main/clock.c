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

/**************************************************************
 *
 *	Macros
 *
 ***************************************************************/
#define TICKS_TO_NEXT_MINUTE(x) (pdMS_TO_TICKS(((int)60 - x) * 1000U))

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void update_time_and_timer(TimerHandle_t xTimer);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static time_t now;
static struct tm timeinfo;
static TickType_t timer_next_ticks_value;
static TimerHandle_t one_minute_timer_handle;
static const char *current_timezone;
static const char *default_timezone = "GMT0";

/******************************************************************************************************************
 *
 * Clock task
 *
 ******************************************************************************************************************/
void Clock_Task(void *arg){

	// set default timezone if current is not set
	if(NULL == current_timezone){

		setenv("TZ", default_timezone, 1);
		tzset();
	}


	// create 1 minute timer
	one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(1000), pdFALSE, NULL, update_time_and_timer);
	assert(one_minute_timer_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, CLOCK_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// read system time to global variables + set and start one minute timer
	update_time_and_timer(NULL);

	vTaskDelete(NULL);
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* inform clock that something has changed an it should update itselfs */
void Clock_TimeUpdated(void){

	update_time_and_timer(NULL);
}

/* inform clock that timezone has been obtained */
void Clock_UpdateTimezone(const char *TimezoneString){

	if(current_timezone == TimezoneString) return;

	current_timezone = TimezoneString;

	if(NULL != TimezoneString){

		setenv("TZ", current_timezone, 1);
		tzset();
		update_time_and_timer(NULL);
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* refresh time value on the screen */
static void update_time_and_timer(TimerHandle_t xTimer){

	// stop timer to set it with new values
	xTimerStop(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	// get current system time
	time(&now);
	localtime_r(&now, &timeinfo);

	// if time has been not set yet request sntp sync and exit
	if(timeinfo.tm_year <= (int)(2020U - 1900U)){

		OnlineRequest_Send(ONLINEREQ_CLOCK_UPDATE, NULL);
		xTimerChangePeriod(one_minute_timer_handle, pdMS_TO_TICKS(5000), pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
		xTimerStart(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
		return;
	}

	// calculate remaining seconds to call timer right after new minute value
	timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(timeinfo.tm_sec);

	// set the timer with calculated value and start it
	xTimerChangePeriod(one_minute_timer_handle, timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	// if no timezone is set, request timezone update
	if(NULL == current_timezone){

		OnlineRequest_Send(ONLINEREQ_TIMEZONE_UPDATE, NULL);
	}

	// if the time for daily sync has come request sntp sync
	if((CLOCK_REFRESH_HOUR == timeinfo.tm_hour) && (CLOCK_RESFRESH_MINUTE == timeinfo.tm_min)){

		OnlineRequest_Send(ONLINEREQ_CLOCK_UPDATE, NULL);
	}

//	if(CLOCK_RESFRESH_MINUTE == timeinfo.tm_min){
//
//		OnlineRequest_Send(ONLINEREQ_WEATHER_UPDATE, NULL);
//	}

	OnlineRequest_Send(ONLINEREQ_WEATHER_UPDATE, NULL);

	// report new time value to UI
	UI_ReportEvt(UI_EVT_TIME_CHANGED, &timeinfo);
}
