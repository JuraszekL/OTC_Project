#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_sntp.h"

#include "main.h"
#include "wifi.h"
#include "clock.h"
#include "online_requests.h"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef void (*online_request)(void *arg);

struct online_request_queue_data {

	OnlineRequest_Type_t type;
	void *arg;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void sntp_initialize(void);
static void time_sync_notification_cb(struct timeval *tv);

static void time_update_request(void *arg);
static void timezone_update_request(void *arg);
static void weather_update_request(void *arg);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
//enum { timezone = 0, weather };
//
//static void http_request_timezone(void);
//static esp_err_t http_event_handler(esp_http_client_event_t *evt);

const online_request requests_tab[] = {

		[ONLINEREQ_CLOCK_UPDATE] = time_update_request,
		[ONLINEREQ_TIMEZONE_UPDATE] = timezone_update_request,
		[ONLINEREQ_WEATHER_UPDATE] = weather_update_request,
};

static QueueHandle_t online_requests_queue_handle;

/******************************************************************************************************************
 *
 * Online Requests task
 *
 ******************************************************************************************************************/
void OnlineRequests_Task(void *arg){

	EventBits_t bits = 0;
	BaseType_t ret;
	struct online_request_queue_data data;

	// create online requests queue
	online_requests_queue_handle = xQueueCreate(3U, sizeof(struct online_request_queue_data));
	assert(online_requests_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, ONLINEREQS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	sntp_initialize();

	while(1){

		// wait for new requests
		ret = xQueueReceive(online_requests_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// check if wifi is connected
			bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(ONLINE_REQ_WAIT_WIFI_MS));
			if(bits & WIFI_CONNECTED_BIT){

				// call related function if wifi is connected
				requests_tab[data.type](data.arg);
			}
		}
	}
}


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* request online data */
void OnlineRequest_Send(OnlineRequest_Type_t Type, void *arg){

	struct online_request_queue_data data;

	data.type = Type;
	data.arg = arg;

	// send data to requests queue in OnlineRequests_Task
	xQueueSend(online_requests_queue_handle, &data, pdMS_TO_TICKS(50));
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* initialize sntp */
static void sntp_initialize(void){

	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, SNTP_SERVER_NAME);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
}

/* sntp time synchronised callback */
static void time_sync_notification_cb(struct timeval *tv){

	esp_sntp_stop();
	Clock_TimeUpdated();
}

/* run sntp to update system time */
static void time_update_request(void *arg){

	// start or init sntp
	if(0 == sntp_restart()){

		esp_sntp_init();
	}
}

static void timezone_update_request(void *arg){


}

static void weather_update_request(void *arg){


}

//static void http_request_timezone(void){
//
//	esp_http_client_config_t config = {
//
//			.url = "http://ip-api.com/json/?fields=24851",
//			.event_handler = http_event_handler,
//			.user_data = timezone,
//	};
//
//	esp_http_client_handle_t client = esp_http_client_init(&config);
//
//    esp_err_t err = esp_http_client_perform(client);
//    if (err == ESP_OK) {
//        ESP_LOGI("", "HTTP Status = %d, content_length = %lld",
//                esp_http_client_get_status_code(client),
//                esp_http_client_get_content_length(client));
//    } else {
//        ESP_LOGE("", "HTTP request failed: %s", esp_err_to_name(err));
//    	// set retry
//    }
//}
//
//static esp_err_t http_event_handler(esp_http_client_event_t *evt){
//
//	switch(evt->event_id){
//
//	// retry if error occured
//	case HTTP_EVENT_ERROR:
//		// set retry
//		break;
//
//	// if data have come check what kind of request it was
//	case HTTP_EVENT_ON_DATA:
//		switch((int)evt->user_data){
//
//		// if it was timezone request call timezone parse function
//		case timezone:
//			// parse json
//			break;
//
//		// if it was weather request call weather parse function
//		case weather:
//			// parse json
//			break;
//
//		// do nothing if other
//		default:
//			break;
//		}
//		break;
//
//	// do nothing if other event happened
//	default:
//		break;
//	}
//
//	return ESP_OK;
//}
