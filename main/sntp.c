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
 *	Function prototypes
 *
 ***************************************************************/
static void time_sync_notification_cb(struct timeval *tv);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static time_t now;
static struct tm timeinfo;

/******************************************************************************************************************
 *
 * SNTP task
 *
 ******************************************************************************************************************/
void SNTP_Task(void *arg){

	EventBits_t bits = 0;
	uint8_t is_sntp_running = 0;

	// get current system time
	time(&now);
	localtime_r(&now, &timeinfo);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SNTP_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// send current system time to display
	UI_ReportEvt(UI_EVT_TIME_CHANGED, &timeinfo);

	// intialize sntp
	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, SNTP_SERVER_NAME);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_set_sync_interval(30);

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

/* send new time to display */
static void time_sync_notification_cb(struct timeval *tv){

	now = tv->tv_sec;
	localtime_r(&now, &timeinfo);
	UI_ReportEvt(UI_EVT_TIME_CHANGED, &timeinfo);
}
