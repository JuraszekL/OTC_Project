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
static void basic_data_update_request(void *arg);
static void detailed_data_update_request(void *arg);

//static esp_err_t timezone_update_evt(esp_http_client_event_t *evt);
static esp_err_t http_perform_evt_handler(esp_http_client_event_t *evt);

static int parse_json_weather_basic(cJSON *json, int *weather_code, uint8_t *is_day);
static int parse_json_clock_basic(cJSON *json, int *unix_time, const char **timezone_string);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const online_request requests_tab[] = {

		[ONLINEREQ_BASIC_UPDATE] = basic_data_update_request,
		[ONLINEREQ_DETAILED_UPDATE] = detailed_data_update_request,
};

static QueueHandle_t online_requests_queue_handle;

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
	online_requests_queue_handle = xQueueCreate(1U, sizeof(struct online_request_queue_data));
	assert(online_requests_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, ONLINEREQS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// wait for new requests
		ret = xQueueReceive(online_requests_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// check if wifi is connected
			bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
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
	xQueueSend(online_requests_queue_handle, &data, pdMS_TO_TICKS(500));
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* request update of basic time/weather data */
static void basic_data_update_request(void *arg){

	int a, unix_time;
	char *json_raw = 0, *html_url_buff = 0;
	const char *timezonestr = 0;
	esp_http_client_handle_t client = NULL;
	esp_http_client_config_t config = {0};
	esp_err_t ret;
	cJSON *recieved_json = 0;
	Weather_BasicData_t weather_basic;

	// prepare buffer for url address
	html_url_buff = heap_caps_calloc(sizeof(char), HTML_URL_LENGTH_MAX, MALLOC_CAP_SPIRAM);
	if(0 == html_url_buff) return;

	// set correct url address
	a = sprintf(html_url_buff, "%s%s%s%s%s", HTTP_WEATHER_URL, HTTP_WEATHER_METH_NOW,
			HTTP_WEATHER_PAR_KEY, CONFIG_ESP_WEATHER_API_KEY, HTTP_WEATHER_QUERY);
	if((0 == a) || (sizeof(html_url_buff) == a)) goto cleanup;

	// configure http connection
	config.url = html_url_buff;
	config.event_handler = http_perform_evt_handler;
	config.user_data = &json_raw;	// pointer where recieved data will be stored (allocated dynamically by evt handler)

	// initialize http client
	client = esp_http_client_init(&config);
	if(NULL == client) goto cleanup;

	// perform http data transfer
	ret = esp_http_client_perform(client);
	if((ESP_OK != ret) || (0 == json_raw)) goto cleanup;

	// parse json
	recieved_json = cJSON_Parse(json_raw);
	if(0 == recieved_json) goto cleanup;

	// get the data about weather
	ret = parse_json_weather_basic(recieved_json, &weather_basic.weather_code, &weather_basic.is_day);
	if(0 == ret){

		Weather_EventReport(WEATHER_BASIC_UPDATE, &weather_basic);
	}

	// get the data about time
	ret = parse_json_clock_basic(recieved_json, &unix_time, &timezonestr);
	if(0 == ret){

		Clock_Update(unix_time, timezonestr);
	}

	cleanup:
		if(recieved_json) cJSON_Delete(recieved_json);
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
		if(client) esp_http_client_cleanup(client);
		if(html_url_buff){
			if(heap_caps_get_allocated_size(html_url_buff)) free(html_url_buff);
		}
}

/* get weather rekated data from parsed json */
static int parse_json_weather_basic(cJSON *json, int *weather_code, uint8_t *is_day){

	int a = -1;
	cJSON *current = 0, *isday = 0,
			*condition = 0, *code = 0;

	current = cJSON_GetObjectItemCaseSensitive(json, "current");
	if(0 == current) return a;;

	isday = cJSON_GetObjectItemCaseSensitive(current, "is_day");
	if(0 == isday) return a;;

	if((0 == cJSON_IsNumber(isday)) || (0 > isday->valueint) || (1 < isday->valueint)){

		ESP_LOGE("", "isday = %d", isday->valueint);
		return a;;
	}

	// return value of day/night
	*is_day = (uint8_t)isday->valueint;

	condition = cJSON_GetObjectItemCaseSensitive(current, "condition");
	if(0 == condition) return a;;

	code = cJSON_GetObjectItemCaseSensitive(condition, "code");
	if(0 == code) return a;;

	if((0 == cJSON_IsNumber(code)) || (1000U > code->valueint) || (1282U < code->valueint)){

		ESP_LOGE("", "code = %d", code->valueint);
		return a;;
	}

	// return weather code
	*weather_code = code->valueint;

	a = 0;
	return a;
}

/* get time related data from parsed json */
static int parse_json_clock_basic(cJSON *json, int *unix_time, const char **timezone_string){

	int a = -1, i;
	size_t len = 0;
	cJSON *location = 0, *localtime_epoch = 0, *tz_id = 0;

	location = cJSON_GetObjectItemCaseSensitive(json, "location");
	if(0 == location) return a;

	localtime_epoch = cJSON_GetObjectItemCaseSensitive(location, "localtime_epoch");
	if(0 == localtime_epoch) return a;
	if(0 == cJSON_IsNumber(localtime_epoch)) return a;

	// return unix format time value
	*unix_time = localtime_epoch->valueint;

	tz_id = cJSON_GetObjectItemCaseSensitive(location, "tz_id");
	if(0 == tz_id) return a;
	if (0 == cJSON_IsString(tz_id) || (tz_id->valuestring == 0)) return a;
	len = strnlen(tz_id->valuestring, 64) + 1;

	// find the string in the constant array with timezone names
	for (i = 0; NULL != TimezonesNames[i]; i++){

		if(0 == memcmp(TimezonesNames[i][0], tz_id->valuestring, len)){

			// return unix timezone string
			*timezone_string = TimezonesNames[i][1];
			break;
		}
	}

	a = 0;
	return a;
}

static void detailed_data_update_request(void *arg){


}

/* handler for http events */
static esp_err_t http_perform_evt_handler(esp_http_client_event_t *evt){

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
