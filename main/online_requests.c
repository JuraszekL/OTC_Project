#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_sntp.h"
#include "string.h"

#include "cJSON.h"

#include "main.h"
#include "wifi.h"
#include "clock.h"
#include "weather.h"
#include "online_requests.h"

#define HTML_URL_LENGTH_MAX		1024U

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

static esp_err_t timezone_update_evt(esp_http_client_event_t *evt);
static esp_err_t weather_update_evt(esp_http_client_event_t *evt);

static int weather_parse_json_simple(char *json, int *weather_code, uint8_t * is_day);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const online_request requests_tab[] = {

		[ONLINEREQ_CLOCK_UPDATE] = time_update_request,
		[ONLINEREQ_TIMEZONE_UPDATE] = timezone_update_request,
		[ONLINEREQ_WEATHER_UPDATE] = weather_update_request,
};

static QueueHandle_t online_requests_queue_handle;
static Weather_SimpleData_t weather_simple_data;

extern const char *TimezonesNames[][2];

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

	/********************************************************
	 * SNTP
	 ********************************************************/

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

	/********************************************************
	 * Timezone
	 ********************************************************/

/* send http request to obtain timezone name */
static void timezone_update_request(void *arg){

	esp_http_client_handle_t client = NULL;
	esp_http_client_config_t config = {

			.url = HTTP_TIMEZONE_URL,
			.event_handler = timezone_update_evt,
	};

	client = esp_http_client_init(&config);
	if(NULL == client) return;

	esp_http_client_perform(client);
	esp_http_client_cleanup(client);
}

/* handle the data obtained from http */
static esp_err_t timezone_update_evt(esp_http_client_event_t *evt){

	esp_err_t ret = ESP_OK;
	char *ptr;
	cJSON *recieved_json = 0;
	cJSON *timezone = 0;
	size_t len = 0;
	int a;

	// if data recieved
	if(HTTP_EVENT_ON_DATA == evt->event_id){

		// copy data from http buffer to spiram
		ptr = heap_caps_calloc(sizeof(char), evt->data_len, MALLOC_CAP_SPIRAM);
		if(NULL == ptr) {

			ret = ESP_FAIL;
			goto cleanup;
		}
		memcpy(ptr, evt->data, evt->data_len);

		// parse recieved data as json string
		recieved_json = cJSON_Parse(ptr);
		if(NULL == recieved_json){

			ret = ESP_FAIL;
			goto cleanup;
		}

		// get "timezone" object from parsed json
		timezone = cJSON_GetObjectItemCaseSensitive(recieved_json, "timezone");
		if(NULL == timezone){

			ret = ESP_FAIL;
			goto cleanup;
		}

		// if "timezone" object is a string
	    if (cJSON_IsString(timezone) && (timezone->valuestring != NULL)){

			// get length of the string
			len = strnlen(timezone->valuestring, 64) + 1;

			// find the string in the constant array with timezone names
	    	for (a = 0; NULL != TimezonesNames[a]; a++){

	    		if(0 == memcmp(TimezonesNames[a][0], timezone->valuestring, len)){

	    			// send corresponding tz string to clock
	    			Clock_UpdateTimezone(TimezonesNames[a][1]);
	    			goto cleanup;
	    		}
	    	}

	    	// if corresponding tz not found send null
	    	Clock_UpdateTimezone(NULL);
	   }

	    cleanup:
			if(recieved_json) cJSON_Delete(recieved_json);
			if(ptr){
				if(heap_caps_get_allocated_size(ptr)) heap_caps_free(ptr);
			}
	}

	return ret;
}


	/********************************************************
	 * Weather
	 ********************************************************/
static void weather_update_request(void *arg){

    int a;
    char *json_raw = 0, *html_url_buff = 0;
	esp_http_client_handle_t client = NULL;
    esp_http_client_config_t config = {0};
    esp_err_t ret;
//    Weather_SimpleData_t data = {0};

    // prepare buffer for url address
    html_url_buff = heap_caps_calloc(sizeof(char), HTML_URL_LENGTH_MAX, MALLOC_CAP_SPIRAM);
    if(0 == html_url_buff) return;

    // set correct url address
    a = sprintf(html_url_buff, "%s%s%s%s%s", HTTP_WEATHER_URL, HTTP_WEATHER_METH_NOW,
    		HTTP_WEATHER_PAR_KEY, CONFIG_ESP_WEATHER_API_KEY, HTTP_WEATHER_QUERY);
    if((0 == a) || (sizeof(html_url_buff) == a)) goto cleanup;

    // configure http connection
    config.url = html_url_buff;
    config.event_handler = weather_update_evt;
    config.user_data = &json_raw;	// pointer where recieved data will be stored (allocated dynamically by evt handler)

    // initialize http client
    client = esp_http_client_init(&config);
    if(NULL == client) goto cleanup;

    // perform http data transfer
    ret = esp_http_client_perform(client);
    if((ESP_OK != ret) || (0 == json_raw)) goto cleanup;

    // parse json
    ret = weather_parse_json_simple(json_raw, &weather_simple_data.weather_code, &weather_simple_data.is_day);
    if(0 == ret){

//    	ESP_LOGI("", "is_day = %d, weather_code = %d", weather_simple_data.is_day, weather_simple_data.weather_code);
    	Weather_EventReport(WEATHER_SIMPLE_UPDATE, &weather_simple_data);
    }
//    else{
//
//    	ESP_LOGE("", "parse json error");
//    }

    cleanup:
		if(html_url_buff){
			if(heap_caps_get_allocated_size(html_url_buff)) free(html_url_buff);
		}
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
		if(client) esp_http_client_cleanup(client);
}

static esp_err_t weather_update_evt(esp_http_client_event_t *evt){

	size_t rcvd_data_len = 0;
	char **ptr = (char **)evt->user_data;
	esp_err_t ret = ESP_OK;

	switch(evt->event_id){

	// free memory if error occured
		case HTTP_EVENT_ERROR:
			if(0 != *ptr){

				free(*ptr);
				*ptr = 0;
			}
			break;

	// recieve the data from http
		case HTTP_EVENT_ON_DATA:
			if(0 != *ptr){

				// check size of allocated memory
				rcvd_data_len = heap_caps_get_allocated_size(*ptr);
			}
			if(0 == rcvd_data_len){

				// allocate memory if nothing has been allocated yet
				*ptr = heap_caps_calloc(sizeof(char), evt->data_len, MALLOC_CAP_SPIRAM);
			}
			else{

				// realocate memory for new data
				*ptr = heap_caps_realloc(*ptr, (rcvd_data_len + evt->data_len), MALLOC_CAP_SPIRAM);
			}
			if(0 == *ptr) return ESP_FAIL;

			// copy new data to allocated memory
			memcpy(*ptr + rcvd_data_len, evt->data, evt->data_len);
			break;

		default:
			break;
	}

	return ret;
}

static int weather_parse_json_simple(char *json, int *weather_code, uint8_t *is_day){

	int a = -1;
	cJSON *recieved_json = 0, *current = 0, *isday = 0,
			*condition = 0, *code = 0;

	recieved_json = cJSON_Parse(json);
	if(0 == recieved_json) goto cleanup;

	current = cJSON_GetObjectItemCaseSensitive(recieved_json, "current");
	if(0 == current) goto cleanup;

	isday = cJSON_GetObjectItemCaseSensitive(current, "is_day");
	if(0 == isday) goto cleanup;

	if((0 == cJSON_IsNumber(isday)) || (0 > isday->valueint) || (1 < isday->valueint)){

		ESP_LOGE("", "isday = %d", isday->valueint);
		goto cleanup;
	}
	*is_day = (uint8_t)isday->valueint;

	condition = cJSON_GetObjectItemCaseSensitive(current, "condition");
	if(0 == condition) goto cleanup;

	code = cJSON_GetObjectItemCaseSensitive(condition, "code");
	if(0 == code) goto cleanup;

	if((0 == cJSON_IsNumber(code)) || (1000U > code->valueint) || (1282U < code->valueint)){

		ESP_LOGE("", "isday = %d", isday->valueint);
		goto cleanup;
	}
	*weather_code = code->valueint;

	a = 0;

	cleanup:
		if(recieved_json) cJSON_Delete(recieved_json);
		return a;
}
