#include "main.h"
#include "esp_http_client.h"
#include "esp_sntp.h"
#include "string.h"
#include <math.h>

#include "cJSON.h"

/**************************************************************
 * HTTP API
 ***************************************************************/
#define HTTP_WEATHER_URL				"http://api.weatherapi.com/v1"
#define HTTP_WEATHER_METH_NOW			"/current.json?"
#define HTTP_WEATHER_METH_FORECAST		"/forecast.json?"
#define HTTP_WEATHER_PAR_KEY			"key="
#define HTTP_WEATHER_QUERY				"&q=auto:ip"

/**************************************************************
 * SNTP server
 ***************************************************************/
#define SNTP_SERVER_NAME				"ntp1.tp.pl"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
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

static void request_time_update(void *arg);
static void request_basic_data_update(void *arg);
static void request_detailed_weather_update(void *arg);

static int http_get_weather_json(const char *method, char **json_raw, cJSON **recieved_json);
static int http_perform_evt_handler(esp_http_client_event_t *evt);

static int parse_json_icon(cJSON *json, char **icon_path);

static int parse_json_timezone_basic(cJSON *json, const char **timezone_string);
static int parse_json_current_weather_icon(cJSON *json, char **icon_path);
static int parse_json_current_weather_values(cJSON *json, int *temp, int *press, int *hum);

static int parse_json_forecast_day_0(cJSON *json, cJSON **_day_0);
static int parse_json_forecast_weather_icon(cJSON *json, char **icon_path);
static int parse_json_forecast_city_country(cJSON *json, char **city_name, char **country_name);
static int parse_json_forecast_avg_temp(cJSON *json, int *avg_temp);
static int parse_json_forecast_min_max_temp(cJSON *json, int *min_temp, int *max_temp);
static int parse_json_forecast_sunrise_sunset_time(cJSON *json, char **sunrise_time, char **sunset_time);
static int parse_json_forecast_recip_percent_rain(cJSON *json, int *recip_rain, int *percent_rain);
static int parse_json_forecast_avg_max_wind(cJSON *json, int *avg_wind, int *max_wind);
static int parse_json_forecast_recip_percent_snow(cJSON *json, int *recip_snow, int *percent_snow);

static int convert_time_string_format(char *from, char **to);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const online_request requests_tab[] = {

		[ONLINEREQ_BASIC_UPDATE] = request_basic_data_update,
		[ONLINEREQ_TIME_UPDATE] = request_time_update,
		[ONLINEREQ_DETAILED_UPDATE] = request_detailed_weather_update,
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

	BaseType_t ret;
	struct online_request_queue_data data;

	// create online requests queue
	online_requests_queue_handle = xQueueCreate(3U, sizeof(struct online_request_queue_data));
	assert(online_requests_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, ONLINEREQS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	sntp_initialize();

	// wait until wifi is connected
	Wifi_WaitUntilIsConnected(portMAX_DELAY);

	while(1){

		// wait for new requests
		ret = xQueueReceive(online_requests_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// check if wifi is connected again
			if(true == Wifi_WaitUntilIsConnected(TIMEOUT_MS)){

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

/* run sntp to update system time */
static void request_time_update(void *arg){

	// start or init sntp
	if(0 == sntp_restart()){

		esp_sntp_init();
	}
}

/* sntp time synchronised callback */
static void sntp_sync_cb(struct timeval *tv){

	esp_sntp_stop();
	TimeUpdated();
}

/*****************************
 * Online requests
 * ***************************/

/* request update of basic time/weather data */
static void request_basic_data_update(void *arg){

	char *json_raw = 0;
	const char *timezonestr = 0;
	int ret;
	cJSON *recieved_json = 0;
	UI_BasicWeatherValues_t *data = 0;

	// get data from http
	ret = http_get_weather_json(HTTP_WEATHER_METH_NOW, &json_raw, &recieved_json);
	if(0 != ret) goto error;

	// allocate memory for output values
	data = malloc(sizeof(UI_BasicWeatherValues_t));
	if(0 == data) goto error;

	// get the weather icon path
	ret = parse_json_current_weather_icon(recieved_json, &data->icon_path);
	if(0 != ret) goto error;

	// get weather values
	ret = parse_json_current_weather_values(recieved_json, &data->temp, &data->press, &data->hum);
	if(0 != ret) goto error;

	// get the timezone string (tz)
	ret = parse_json_timezone_basic(recieved_json, &timezonestr);
	if(0 != ret) goto error;

	// if everything was fine use obtained data to:

	// set timezone
	Timezone_Update(timezonestr);
	// update weather data on main screen
	UI_ReportEvt(UI_EVT_BASIC_WEATHER_UPDATE, data);

	// cleanup
	cJSON_Delete(recieved_json);
	free(json_raw);
	return;

	error:
		if(recieved_json) cJSON_Delete(recieved_json);
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
		if(data){
			if(heap_caps_get_allocated_size(data)) free(data);
		}
}

/* request update of detailed weather data */
static void request_detailed_weather_update(void *arg){

	int ret;
	char *json_raw = 0;
	cJSON *recieved_json = 0, *day_0 = 0;
	UI_DetailedWeatherValues_t *data = 0;

	// get data from http
	ret = http_get_weather_json(HTTP_WEATHER_METH_FORECAST, &json_raw, &recieved_json);
	if(0 != ret) goto error;

	// get pointer to data of today's weather
	ret = parse_json_forecast_day_0(recieved_json, &day_0);
	if(0 != ret) goto error;

	// allocate memory for output values
	data = malloc(sizeof(UI_DetailedWeatherValues_t));
	if(0 == data) goto error;

	// get name of city and country
	ret = parse_json_forecast_city_country(recieved_json, &data->city_name, &data->country_name);
	if(0 != ret) goto error;

	// get weather icon path
	ret = parse_json_forecast_weather_icon(day_0, &data->icon_path);
	if(0 != ret) goto error;

	// get avg temperature
	ret = parse_json_forecast_avg_temp(day_0, &data->average_temp);
	if(0 != ret) goto error;

	// get min/max temperature
	ret = parse_json_forecast_min_max_temp(day_0, &data->min_temp, &data->max_temp);
	if(0 != ret) goto error;

	ret = parse_json_forecast_sunrise_sunset_time(day_0, &data->sunrise_time, &data->sunset_time);
	if(0 != ret) goto error;

	ret = parse_json_forecast_recip_percent_rain(day_0, &data->recip_rain, &data->percent_rain);
	if(0 != ret) goto error;

	ret = parse_json_forecast_avg_max_wind(recieved_json, &data->wind_avg, &data->wind_max);
	if(0 != ret) goto error;

	ret = parse_json_forecast_recip_percent_snow(day_0, &data->recip_snow, &data->percent_snow);
	if(0 != ret) goto error;

	// if everything was fine send obtained data to weather screen
	UI_ReportEvt(UI_EVT_DETAILED_WEATHER_UPDATE, data);

	// cleanup
	cJSON_Delete(recieved_json);
	free(json_raw);
	return;

	error:
		if(recieved_json) cJSON_Delete(recieved_json);
		if(json_raw){
			if(heap_caps_get_allocated_size(json_raw)) free(json_raw);
		}
		if(data){
			if(heap_caps_get_allocated_size(data)) free(data);
		}
}

/*****************************
 * HTTP operations
 * ***************************/

/* general function to get json from weatherapi.com */
static int http_get_weather_json(const char *method, char **json_raw, cJSON **recieved_json){

	int len, ret;
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
	if((0 != ret) || (0 == *json_raw)) goto error;

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
static int http_perform_evt_handler(esp_http_client_event_t *evt){

	char **ptr = (char **)evt->user_data;

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
			return -1;
		}

		// copy new data to allocated memory
		memcpy(*ptr + rcvd_data_len, evt->data, evt->data_len);
		rcvd_data_len += evt->data_len;
		break;

	default:
		break;
	}

	return 0;
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

/* get current weather values from parsed json */
static int parse_json_current_weather_values(cJSON *json, int *temp, int *press, int *hum){

	int a = -1;
	cJSON *current = 0, *temp_c = 0, *pressure_mb = 0, *humidity = 0;

	current = cJSON_GetObjectItemCaseSensitive(json, "current");
	if(0 == current) return a;

	temp_c = cJSON_GetObjectItemCaseSensitive(current, "temp_c");
	if(0 == temp_c) return a;
	if(0 == cJSON_IsNumber(temp_c)) return a;
	*temp = temp_c->valueint;

	pressure_mb = cJSON_GetObjectItemCaseSensitive(current, "pressure_mb");
	if(0 == pressure_mb) return a;
	if(0 == cJSON_IsNumber(pressure_mb)) return a;
	*press = pressure_mb->valueint;

	humidity = cJSON_GetObjectItemCaseSensitive(current, "humidity");
	if(0 == humidity) return a;
	if(0 == cJSON_IsNumber(humidity)) return a;
	*hum = humidity->valueint;

	a = 0;
	return a;
}

/* get pointer to current day's json */
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

/* get weather icon path from forecast weather json */
static int parse_json_forecast_weather_icon(cJSON *json, char **icon_path){

	int a = -1;
	cJSON *day = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return a;

	if(0 != parse_json_icon(day, icon_path)) return a;

	a = 0;
	return a;
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

/* get average temperature from json */
static int parse_json_forecast_avg_temp(cJSON *json, int *avg_temp){

	cJSON *day = 0, *avgtemp_c = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return -1;

	avgtemp_c = cJSON_GetObjectItemCaseSensitive(day, "avgtemp_c");
	if(0 == avgtemp_c) return -1;
	if(0 == cJSON_IsNumber(avgtemp_c)) return -1;
	*avg_temp = (int)round(avgtemp_c->valuedouble);

	return 0;
}

/* get minimum and maximum temperature from json */
static int parse_json_forecast_min_max_temp(cJSON *json, int *min_temp, int *max_temp){

	cJSON *day = 0, *mintemp_c = 0, *maxtemp_c = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return -1;

	mintemp_c = cJSON_GetObjectItemCaseSensitive(day, "mintemp_c");
	if(0 == mintemp_c) return -1;
	if(0 == cJSON_IsNumber(mintemp_c)) return -1;
	*min_temp = (int)round(mintemp_c->valuedouble);

	maxtemp_c = cJSON_GetObjectItemCaseSensitive(day, "maxtemp_c");
	if(0 == maxtemp_c) return -1;
	if(0 == cJSON_IsNumber(maxtemp_c)) return -1;
	*max_temp = (int)round(maxtemp_c->valuedouble);

	return 0;
}

/* get sunrise and sunset time as strings from json */
static int parse_json_forecast_sunrise_sunset_time(cJSON *json, char **sunrise_time, char **sunset_time){

	cJSON *astro = 0, *sunrise = 0, *sunset = 0;

	astro = cJSON_GetObjectItemCaseSensitive(json, "astro");
	if(0 == astro) return -1;

	sunrise = cJSON_GetObjectItemCaseSensitive(astro, "sunrise");
	if(0 == sunrise) goto error;
	if (0 == cJSON_IsString(sunrise) || (sunrise->valuestring == 0)) goto error;

	if(0 != convert_time_string_format(sunrise->valuestring, sunrise_time)) goto error;

	sunset = cJSON_GetObjectItemCaseSensitive(astro, "sunset");
	if(0 == sunset) goto error;
	if (0 == cJSON_IsString(sunset) || (sunset->valuestring == 0)) goto error;

	if(0 != convert_time_string_format(sunset->valuestring, sunset_time)) goto error;

	return 0;

	error:
		if(*sunrise_time){
			if(heap_caps_get_allocated_size(*sunrise_time)) free(*sunrise_time);
		}
		if(*sunset_time){
			if(heap_caps_get_allocated_size(*sunset_time)) free(*sunset_time);
		}
		return -1;
}

/* get total precipitation and percentage chance of rain from json  */
static int parse_json_forecast_recip_percent_rain(cJSON *json, int *recip_rain, int *percent_rain){

	cJSON *day = 0, *totalprecip_mm = 0, *daily_chance_of_rain = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return -1;

	totalprecip_mm = cJSON_GetObjectItemCaseSensitive(day, "totalprecip_mm");
	if(0 == totalprecip_mm) return -1;
	if(0 == cJSON_IsNumber(totalprecip_mm)) return -1;
	*recip_rain = (int)round(totalprecip_mm->valuedouble);

	daily_chance_of_rain = cJSON_GetObjectItemCaseSensitive(day, "daily_chance_of_rain");
	if(0 == daily_chance_of_rain) return -1;
	if(0 == cJSON_IsNumber(daily_chance_of_rain)) return -1;
	*percent_rain = daily_chance_of_rain->valuedouble;

	return 0;
}

/* get average and maximum wind speed from recieved json */
static int parse_json_forecast_avg_max_wind(cJSON *json, int *avg_wind, int *max_wind){

	cJSON *current = 0, *wind_kph = 0, *gust_kph = 0;

	current = cJSON_GetObjectItemCaseSensitive(json, "current");
	if(0 == current) return -1;

	wind_kph = cJSON_GetObjectItemCaseSensitive(current, "wind_kph");
	if(0 == wind_kph) return -1;
	if(0 == cJSON_IsNumber(wind_kph)) return -1;
	*avg_wind = (int)round(wind_kph->valuedouble);

	gust_kph = cJSON_GetObjectItemCaseSensitive(current, "gust_kph");
	if(0 == gust_kph) return -1;
	if(0 == cJSON_IsNumber(gust_kph)) return -1;
	*max_wind = gust_kph->valuedouble;

	return 0;
}

/* get total precipitation and percentage chance of snow from json  */
static int parse_json_forecast_recip_percent_snow(cJSON *json, int *recip_snow, int *percent_snow){

	cJSON *day = 0, *totalsnow_cm = 0, *daily_chance_of_snow = 0;

	day = cJSON_GetObjectItemCaseSensitive(json, "day");
	if(0 == day) return -1;

	totalsnow_cm = cJSON_GetObjectItemCaseSensitive(day, "totalsnow_cm");
	if(0 == totalsnow_cm) return -1;
	if(0 == cJSON_IsNumber(totalsnow_cm)) return -1;
	*recip_snow = (int)round(totalsnow_cm->valuedouble);

	daily_chance_of_snow = cJSON_GetObjectItemCaseSensitive(day, "daily_chance_of_snow");
	if(0 == daily_chance_of_snow) return -1;
	if(0 == cJSON_IsNumber(daily_chance_of_snow)) return -1;
	*percent_snow = daily_chance_of_snow->valuedouble;

	return 0;
}

/* Convert string from format:
 * 	09:13 AM
 * to format:
 * 	21:13
 * */
static int convert_time_string_format(char *from, char **to){

	int h, m, res;
	char ampm[3];
	size_t len;

	len = strnlen(from, 12);
	if(12 == len) return -1;

	// parse string
	res = sscanf(from, "%d:%d %s", &h, &m, ampm);
	if(3 != res) return -1;

	// check if PM
	if(0 == memcmp(ampm, "PM", 2)){

		h += 12;
	}
	else if(0 != memcmp(ampm, "AM", 2)) return -1; // make sure that only AM or PM value is stored

	*to = malloc(6);
	if(0 == *to) return -1;

	// create new string
	sprintf(*to, "%02d:%02d", h, m);

	return 0;
}
