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
#include "ui_task.h"
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
static void sntp_sync_cb(struct timeval *tv);

static void basic_data_update_request(void *arg);
static void time_sync_request(void *arg);
static void detailed_data_update_request(void *arg);

static int http_get_weather_json(const char *method, char **json_raw, cJSON **recieved_json);
static esp_err_t http_perform_evt_handler(esp_http_client_event_t *evt);

static int parse_json_icon(cJSON *json, char **icon_path);
static int parse_json_current_weather_icon(cJSON *json, char **icon_path);
static int parse_json_forecast_weather_icon(cJSON *json, char **icon_path);

static int parse_json_current_weather_values(cJSON *json, UI_WeatherValues_t **icon_values);
static int parse_json_timezone_basic(cJSON *json, const char **timezone_string);

static int parse_json_forecast_day_0(cJSON *json, cJSON **_day_0);
static int parse_json_forecast_city_country(cJSON *json, char **city_name, char **country_name);



/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const online_request requests_tab[] = {

		[ONLINEREQ_BASIC_UPDATE] = basic_data_update_request,
		[ONLINEREQ_TIME_UPDATE] = time_sync_request,
		[ONLINEREQ_DETAILED_UPDATE] = detailed_data_update_request,
};

static QueueHandle_t online_requests_queue_handle;
static int rcvd_data_len;

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

	// wait until wifi is connected
	bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	while(1){

		// wait for new requests
		ret = xQueueReceive(online_requests_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// check if wifi is connected again
			bits = xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(TIMEOUT_MS));
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

/*****************************
 * SNTP functions
 * ***************************/

/* initialize sntp */
static void sntp_initialize(void){

	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, SNTP_SERVER_NAME);
	sntp_set_time_sync_notification_cb(sntp_sync_cb);
}

/* sntp time synchronised callback */
static void sntp_sync_cb(struct timeval *tv){

	esp_sntp_stop();
	TimeUpdated();
}

/* run sntp to update system time */
static void time_sync_request(void *arg){

	// start or init sntp
	if(0 == sntp_restart()){

		esp_sntp_init();
	}
}

/*****************************
 * Online requests
 * ***************************/

/* request update of basic time/weather data */
static void basic_data_update_request(void *arg){

	char *json_raw = 0, *weather_icon_path = 0;
	const char *timezonestr = 0;
	int ret;
	cJSON *recieved_json = 0;
	UI_WeatherValues_t *weather_values = 0;

	if(0 != http_get_weather_json(HTTP_WEATHER_METH_NOW, &json_raw, &recieved_json)) goto cleanup;

	// set the weather icon
	ret = parse_json_current_weather_icon(recieved_json, &weather_icon_path);
	if(0 == ret){

		UI_ReportEvt(UI_EVT_BASIC_WEATHER_ICON_UPDATE, weather_icon_path);
	}

	ret = parse_json_current_weather_values(recieved_json, &weather_values);
	if(0 == ret){

		UI_ReportEvt(UI_EVT_BASIC_WEATHER_VALUES_UPDATE, weather_values);
	}

	// set the timezone string (tz)
	ret = parse_json_timezone_basic(recieved_json, &timezonestr);
	if(0 == ret){

		Timezone_Update(timezonestr);
	}

	cleanup:
		if(recieved_json) cJSON_Delete(recieved_json);
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
}

static void detailed_data_update_request(void *arg){

	int ret;
	char *json_raw = 0, *city_name = 0, *country_name = 0, *icon_path = 0;
	cJSON *recieved_json = 0, *day_0 = 0;

	if(0 != http_get_weather_json(HTTP_WEATHER_METH_FORECAST, &json_raw, &recieved_json)) goto cleanup;

	if(0 != parse_json_forecast_day_0(recieved_json, &day_0)) goto cleanup;

	ret = parse_json_forecast_city_country(recieved_json, &city_name, &country_name);
	if(0 == ret){

		UI_ReportEvt(UI_EVT_WEATHER_CITY_UPDATE, city_name);
		UI_ReportEvt(UI_EVT_WEATHER_COUNTRY_UPDATE, country_name);
	}

	ret = parse_json_forecast_weather_icon(day_0, &icon_path);
	if(0 == ret){

		UI_ReportEvt(UI_EVT_WEATHER_ICON_UPDATE, icon_path);
	}

	cleanup:
		if(recieved_json) cJSON_Delete(recieved_json);
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
}

/*****************************
 * HTTP operations
 * ***************************/

/* general function to get json from weatherapi.com */
static int http_get_weather_json(const char *method, char **json_raw, cJSON **recieved_json){

	int len;
	esp_err_t ret;
	char *html_url_buff = 0;
	esp_http_client_handle_t client = NULL;
	esp_http_client_config_t config = {0};

	// allocate buffer for url address
	html_url_buff = heap_caps_calloc(sizeof(char), HTML_URL_LENGTH_MAX, MALLOC_CAP_SPIRAM);
	if(0 == html_url_buff) goto error;

	// prepare correct url address
	len = sprintf(html_url_buff, "%s%s%s%s%s", HTTP_WEATHER_URL, method,
			HTTP_WEATHER_PAR_KEY, CONFIG_ESP_WEATHER_API_KEY, HTTP_WEATHER_QUERY);
	if((0 == len) || (HTML_URL_LENGTH_MAX == len)) goto error;

	// configure http connection
	config.url = html_url_buff;
	config.event_handler = http_perform_evt_handler;
	config.user_data = json_raw;	// pointer where recieved data will be stored (allocated dynamically by evt handler)

	// initialize http client
	client = esp_http_client_init(&config);
	if(NULL == client) goto error;

	// perform http data transfer
	ret = esp_http_client_perform(client);
	if((ESP_OK != ret) || (0 == *json_raw)) goto error;

	// parse json
	*recieved_json = cJSON_Parse(*json_raw);
	if(0 == *recieved_json) goto error;

	esp_http_client_cleanup(client);
	free(html_url_buff);
	return 0;

	error:
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
		if(client) esp_http_client_cleanup(client);
		if(html_url_buff){
			if(heap_caps_get_allocated_size(html_url_buff)) free(html_url_buff);
		}
		return -1;
}

/* handler for http events */
static esp_err_t http_perform_evt_handler(esp_http_client_event_t *evt){

	char **ptr = (char **)evt->user_data;
	esp_err_t ret = ESP_OK;

	switch(evt->event_id){

	// free memory if error occured
	case HTTP_EVENT_ERROR:
		if(0 != *ptr){

			ESP_LOGE("", "http error!");
			free(*ptr);
			*ptr = 0;
			rcvd_data_len = 0;
		}
		break;

	// recieve the data from http
	case HTTP_EVENT_ON_DATA:

		if(0 == *ptr){

			// first call of function because *ptr is NULL
			rcvd_data_len = 0;

			// allocate memory if nothing has been allocated yet
			*ptr = heap_caps_calloc(sizeof(char), evt->data_len, (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
		}

		else{

			// realocate memory for new data
			*ptr = heap_caps_realloc(*ptr, (rcvd_data_len + evt->data_len), (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
		}
		if(0 == *ptr) {

			rcvd_data_len = 0;
			return ESP_FAIL;
		}

		// copy new data to allocated memory
		memcpy(*ptr + rcvd_data_len, evt->data, evt->data_len);
		rcvd_data_len += evt->data_len;
		break;

	default:
		break;
	}

	return ret;
}

/*****************************
 * JSON operations
 * ***************************/

/* find the icon file path from JSON, general */
static int parse_json_icon(cJSON *json, char **icon_path){

	int a = -1, i, j = 0;
	cJSON *condition = 0, *icon = 0;
	size_t len = 0;

	condition = cJSON_GetObjectItemCaseSensitive(json, "condition");
	if(0 == condition) return a;

	icon = cJSON_GetObjectItemCaseSensitive(condition, "icon");	// online icon path cdn.weatherapi.com/weather/64x64/day/116.png
	if(0 == icon) return a;;
	if((0 == cJSON_IsString(icon)) || (icon->valuestring == 0)) return a;

	// get whole string lenght
	len = strnlen(icon->valuestring, 256) + 1;
	if(256 == len) return a;

	// find second '/' character from back of obtained address
	// //cdn.weatherapi.com/weather/64x64/day/116.png
	// result should point to the '/' character right after 64 number
	for(i = len; i >= 0; i--){

		if('/' == icon->valuestring[i]){

			j++;
			if(2 == j) break;
		}
	}

	// get string lenht from second '/' character
	len = strnlen(&icon->valuestring[i], 64) + 1;
	if(64 == len) return a;

	// allocate buffer and copy the shortened string
	*icon_path = malloc(len);
	if(0 == *icon_path) return a;

	memcpy(*icon_path, &icon->valuestring[i], len);	// result = /day/116.png

	a = 0;
	return a;
}

/* get weather icon path from current weather json */
static int parse_json_current_weather_icon(cJSON *json, char **icon_path){

	int a = -1;
	cJSON *current = 0;

	current = cJSON_GetObjectItemCaseSensitive(json, "current");
	if(0 == current) return a;

	if(0 != parse_json_icon(current, icon_path)) return a;

	a = 0;
	return a;
}

/* get weather icon path from current weather json */
static int parse_json_forecast_weather_icon(cJSON *json, char **icon_path){

	int a = -1;
	cJSON *day = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return a;

	if(0 != parse_json_icon(day, icon_path)) return a;

	a = 0;
	return a;
}

/* get weather values from parsed json */
static int parse_json_current_weather_values(cJSON *json, UI_WeatherValues_t **icon_values){

	int a = -1, temp, press, hum;
	cJSON *current = 0, *temp_c = 0, *pressure_mb = 0, *humidity = 0;
	UI_WeatherValues_t *values = 0;

	current = cJSON_GetObjectItemCaseSensitive(json, "current");
	if(0 == current) return a;

	temp_c = cJSON_GetObjectItemCaseSensitive(current, "temp_c");
	if(0 == temp_c) return a;
	if(0 == cJSON_IsNumber(temp_c)) return a;
	temp = temp_c->valueint;

	pressure_mb = cJSON_GetObjectItemCaseSensitive(current, "pressure_mb");
	if(0 == pressure_mb) return a;
	if(0 == cJSON_IsNumber(pressure_mb)) return a;
	press = pressure_mb->valueint;

	humidity = cJSON_GetObjectItemCaseSensitive(current, "humidity");
	if(0 == humidity) return a;
	if(0 == cJSON_IsNumber(humidity)) return a;
	hum = humidity->valueint;

	values = malloc(sizeof(UI_WeatherValues_t));
	if(0 == values) return a;
	values->temp = temp;
	values->press = press;
	values->hum = hum;
	*icon_values = values;

	a = 0;
	return a;
}

/* get timezone name from parsed json */
static int parse_json_timezone_basic(cJSON *json, const char **timezone_string){

	int a = -1, i;
	size_t len = 0;
	cJSON *location = 0, *tz_id = 0;

	location = cJSON_GetObjectItemCaseSensitive(json, "location");
	if(0 == location) return a;

	tz_id = cJSON_GetObjectItemCaseSensitive(location, "tz_id");	// timezone Europe/Berlin
	if(0 == tz_id) return a;
	if (0 == cJSON_IsString(tz_id) || (tz_id->valuestring == 0)) return a;
	len = strnlen(tz_id->valuestring, 64) + 1;

	// find the string in the constant array with timezone names
	for (i = 0; NULL != TimezonesNames[i]; i++){

		if(0 == memcmp(TimezonesNames[i][0], tz_id->valuestring, len)){

			// return unix timezone string
			*timezone_string = TimezonesNames[i][1];	// result = CET-1CEST,M3.5.0,M10.5.0/3
			break;
		}
	}
	a = 0;
	return a;
}


static int parse_json_forecast_day_0(cJSON *json, cJSON **day_0){

	cJSON *forecast = 0, *forecastday = 0;

	forecast = cJSON_GetObjectItemCaseSensitive(json, "forecast");
	if(0 == forecast) return -1;

	forecastday = cJSON_GetObjectItemCaseSensitive(forecast, "forecastday");
	if(0 == forecastday) return -1;

	*day_0 = cJSON_GetArrayItem(forecastday, 0);
	if(0 == *day_0) return -1;

	return 0;
}

/* get city and country name from recieved json */
static int parse_json_forecast_city_country(cJSON *json, char **city_name, char **country_name){

	size_t len = 0;
	cJSON *location = 0, *name = 0, *country = 0;

	location = cJSON_GetObjectItemCaseSensitive(json, "location");
	if(0 == location) goto error;

	name = cJSON_GetObjectItemCaseSensitive(location, "name");
	if(0 == name) goto error;
	if (0 == cJSON_IsString(name) || (name->valuestring == 0)) goto error;

	len = strnlen(name->valuestring, 256) + 1;
	if(256 == len) goto error;
	*city_name = malloc(len);
	if(0 == *city_name) goto error;
	memcpy(*city_name, name->valuestring, len);

	country = cJSON_GetObjectItemCaseSensitive(location, "country");
	if(0 == country) goto error;
	if (0 == cJSON_IsString(country) || (country->valuestring == 0)) goto error;

	len = strnlen(country->valuestring, 256) + 1;
	if(256 == len) goto error;
	*country_name = malloc(len);
	if(0 == *country_name) goto error;
	memcpy(*country_name, country->valuestring, len);

	return 0;

	error:
		if(*city_name){
			if(heap_caps_get_allocated_size(*city_name)) free(*city_name);
		}
		if(*country_name){
			if(heap_caps_get_allocated_size(*country_name)) free(*country_name);
		}
		return -1;
}

