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

/******************************************************************************************************************
 *
 * SNTP task
 *
 ******************************************************************************************************************/
void SNTP_Task(void *arg){

	EventBits_t bits = 0;
	uint8_t is_sntp_running = 0;

	// create 1 minute timer
	one_minute_timer_handle = xTimerCreate("", pdMS_TO_TICKS(1000), pdFALSE, NULL, one_minute_timer_callback);
	assert(one_minute_timer_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SNTP_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// read system time to global variables + set and start one minute timer
	update_time_and_timer();

	// intialize sntp
	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, SNTP_SERVER_NAME);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_set_sync_interval(SNTP_TIME_REFRESH_PERIOD_SEC);

	while(1){

		// if sntp is stopped
		if(0 == is_sntp_running){

			// wait until wifi is connected
			bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
			if(bits & WIFI_CONNECTED_BIT){

				// start or init sntp
				if(0 == sntp_restart()){

					esp_sntp_init();
				}
				is_sntp_running = 1;
			}
		}
		// if sntp is running
		else if(1 == is_sntp_running){

			// check is wifi is not disconnected
			bits = xEventGroupWaitBits(WifiEvents, WIFI_DISCONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
			if(bits & WIFI_DISCONNECTED_BIT){

				// stop sntp if is
				esp_sntp_stop();
				is_sntp_running = 0;
			}
		}
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

	xTimerStop(one_minute_timer_handle, pdMS_TO_TICKS(XTIMER_QUEUE_DELAY_MS));
	update_time_and_timer();
}

/* timer callback called right after minute value has changed */
static void one_minute_timer_callback(TimerHandle_t xTimer){

	update_time_and_timer();
}
