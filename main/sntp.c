#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#include "main.h"
#include "ui_task.h"
#include "wifi.h"
#include "sntp.h"

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
static void time_sync_notification_cb(struct timeval *tv);
static void one_minute_timer_callback(TimerHandle_t xTimer);
static void update_time_and_timer(void);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static time_t now;
static struct tm timeinfo;
static TickType_t timer_next_ticks_value;
static TimerHandle_t one_minute_timer_handle;
static TaskHandle_t sntp_task_handle;

static const char timezone[] = "CET-1CEST,M3.5.0,M10.5.0/3";

/******************************************************************************************************************
 *
 * SNTP task
 *
 ******************************************************************************************************************/
void SNTP_Task(void *arg){

	EventBits_t bits = 0;

	// set timezone
	setenv("TZ", timezone, 1);
	tzset();

	// create 1 minute timer
	one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(1000), pdFALSE, NULL, one_minute_timer_callback);
	assert(one_minute_timer_handle);

	sntp_task_handle = xTaskGetCurrentTaskHandle();
	assert(sntp_task_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SNTP_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// read system time to global variables + set and start one minute timer
	update_time_and_timer();

	// intialize sntp
	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, SNTP_SERVER_NAME);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);

	while(1){

		// wait for wifi
		bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
		if(bits & WIFI_CONNECTED_BIT){

			// start or init sntp
			if(0 == sntp_restart()){

				esp_sntp_init();
			}
		}

		// wait for next run
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* refresh time value on the screen */
static void update_time_and_timer(void){

	// get current system time
	time(&now);
	localtime_r(&now, &timeinfo);

	// calculate remaining seconds to call timer right after new minute value
	timer_next_ticks_value = TICKS_TO_NEXT_MINUTE(timeinfo.tm_sec);

	// set the timer with calculated value and start it
	xTimerChangePeriod(one_minute_timer_handle, timer_next_ticks_value, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	xTimerStart(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	// report new time value to UI
	UI_ReportEvt(UI_EVT_TIME_CHANGED, &timeinfo);
}

/* sntp time synchronised callback */
static void time_sync_notification_cb(struct timeval *tv){

	// stop timer to set it with new values
	xTimerStop(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));

	// stop sntp
	esp_sntp_stop();

	// update time
	update_time_and_timer();
}

/* timer callback called right after minute value has changed */
static void one_minute_timer_callback(TimerHandle_t xTimer){

	// update time
	update_time_and_timer();

	// turn on sntp if the time has come
	if((SNTP_REFRESH_HOUR == timeinfo.tm_hour) && (SNTP_RESFRESH_MINUTE == timeinfo.tm_min)){

		xTaskNotifyGive(sntp_task_handle);
	}
}
