#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "lvgl.h"
#include "ui.h"

#include "main.h"
#include "animations.h"
#include "wifi.h"
#include "display.h"
#include "online_requests.h"
#include "ui_wifi_list.h"
//#include "ui_wifi_popup.h"
#include "ui_styles.h"
#include "ui_main_screen.h"
#include "ui_wifi_screen.h"
#include "ui_task.h"

/**************************************************************
 *
 *	Macros
 *
 ***************************************************************/
#define VERSION	("v." VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH)

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef void (*ui_event)(void *arg);

struct ui_queue_data {

	UI_EventType_t type;
	void *arg;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void startup_screen(void);

static void ui_event_wifi_disconnected(void *arg);
static void ui_event_wifi_connected(void *arg);
static void ui_event_wifi_connecting(void *arg);
static void ui_event_wifi_connect_error(void *arg);
static void ui_event_wifi_get_pass(void *arg);
static void ui_event_wifilist_add(void *arg);
static void ui_event_wifilist_clear(void *arg);
static void ui_event_time_changed(void *arg);
static void ui_event_clock_not_sync(void *arg);
static void ui_event_clock_sync(void *arg);
static void ui_event_basic_weather_update(void *arg);
static void ui_event_detailed_weather_update(void *arg);
static void ui_event_main_scr_wifi_btn_clicked(void *arg);
static void ui_event_wifi_scr_back_btn_clicked(void *arg);

static void detailed_weather_update_city_name(char *city_name);
static void detailed_weather_update_country_name(char *country_name);
static void detailed_weather_update_weather_icon(char *icon_path);
static void detailed_weather_update_avg_temp(int avg_temp);
static void detailed_weather_update_min_max_temp(int min_temp, int max_temp);
static void detailed_weather_update_sunrise_sunset_time(char *sunrise_time, char *sunset_time);
static void detailed_weather_update_precip_percent_rain(int precip, int percent);
static void detailed_weather_update_avg_max_wind(int avg, int max);
static void detailed_weather_update_precip_percent_snow(int precip, int percent);

static void set_weather_icon(char *path, lv_obj_t * obj);
//static void set_ap_details(UI_DetailedAPData_t *data);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
const char version[] = {VERSION};
const ui_event event_tab[] = {

		[UI_EVT_WIFI_CONNECTED] = ui_event_wifi_connected,
		[UI_EVT_WIFI_DISCONNECTED] = ui_event_wifi_disconnected,
		[UI_EVT_WIFI_CONNECTING] = ui_event_wifi_connecting,
		[UI_EVT_WIFI_CONNECT_ERROR] = ui_event_wifi_connect_error,
		[UI_EVT_WIFI_GET_PASS] = ui_event_wifi_get_pass,
		[UI_EVT_WIFI_LIST_ADD] = ui_event_wifilist_add,
		[UI_EVT_WIFI_LIST_CLEAR] = ui_event_wifilist_clear,
		[UI_EVT_TIME_CHANGED] = ui_event_time_changed,
		[UI_EVT_CLOCK_NOT_SYNC] = ui_event_clock_not_sync,
		[UI_EVT_CLOCK_SYNC] = ui_event_clock_sync,
		[UI_EVT_BASIC_WEATHER_UPDATE] = ui_event_basic_weather_update,
		[UI_EVT_DETAILED_WEATHER_UPDATE] = ui_event_detailed_weather_update,
		[UI_EVT_MAINSCR_WIFI_BTN_CLICKED] = ui_event_main_scr_wifi_btn_clicked,
		[UI_EVT_WIFISCR_BACK_BTN_CLICKED] = ui_event_wifi_scr_back_btn_clicked,
};

static QueueHandle_t ui_queue_handle;


/******************************************************************************************************************
 *
 * UI task
 *
 ******************************************************************************************************************/
void UI_Task(void *arg){

	BaseType_t ret;
	struct ui_queue_data data;

	// create UI events queue
	ui_queue_handle = xQueueCreate(3U, sizeof(struct ui_queue_data));
	assert(ui_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, UI_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	UI_InitStyles();

	// run startup screen
	startup_screen();

	xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
	UI_MainScreen_Init();
	ui_WeatherDetailsScrren_screen_init();
	UI_WifiScreen_Init();

	UI_MainScreen_Load(2000);
	xSemaphoreGive(LVGL_MutexHandle);

	while(1){

		ret = xQueueReceive(ui_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			ret = xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
			if(pdTRUE == ret){

				event_tab[data.type](data.arg);
		    	xSemaphoreGive(LVGL_MutexHandle);
			}
		}
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

void UI_ReportEvt(UI_EventType_t Type, void *arg){

	struct ui_queue_data data;
	BaseType_t res;

	data.type = Type;
	data.arg = arg;

	res = xQueueSend(ui_queue_handle, &data, pdMS_TO_TICKS(100));
	if(pdPASS != res){

		if(arg){
			if(heap_caps_get_allocated_size(arg)) free(arg);
		}
	}
}


void WetaherScreen_BackButtonClicked(lv_event_t * e){

	UI_ReportEvt(UI_EVT_WIFISCR_BACK_BTN_CLICKED, 0);
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* setup and display startup screend */
static void startup_screen(void){

	// init startup screen
	ui_StartupScreen_screen_init();

	// hide all elements
	lv_obj_set_style_bg_opa(ui_StartupScreenPanel, 0, 0);
	lv_obj_set_style_text_opa(ui_OnlineTableClockLabel, 0, 0);
	lv_obj_set_style_text_opa(ui_ByJuraszekLLabel, 0, 0);
	lv_obj_set_style_shadow_opa(ui_StartupScreenPanel, 0, 0);
	lv_obj_add_flag(ui_LogosImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_VersionLabel, LV_OBJ_FLAG_HIDDEN);

	// set version number to label
	lv_label_set_text(ui_VersionLabel, version);

	// load screen
	lv_disp_load_scr(ui_StartupScreen);

	// start animation
	Anm_InitScr2200msOpa();

	// show image with logos and version label
	vTaskDelay(pdMS_TO_TICKS(2500));
	lv_obj_clear_flag(ui_LogosImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(ui_VersionLabel, LV_OBJ_FLAG_HIDDEN);
}

/*****************************
 * UI event functions
 * ***************************/

static void ui_event_wifi_disconnected(void *arg){

	// set wifi icon in top left corner of main screen
	UI_MainScreen_SetTopIcon(wifi_icon, ICON_NO_WIFI);

	// clear information about connected AP
	UI_WifiScreen_SetApDetails(0);
}

static void ui_event_wifi_connected(void *arg){

	UI_DetailedAPData_t *data = (UI_DetailedAPData_t *)arg;

	// set wifi icon in top left corner of main screen
	UI_MainScreen_SetTopIcon(wifi_icon, ICON_WIFI);

	// create popup with information that wifi is connected
	UI_WifiScreen_Connected(data);

	// set information about connected AP on wifi screen
	UI_WifiScreen_SetApDetails(data);

	// free resources
	if(data){

		if(data->ssid){

			if(heap_caps_get_allocated_size(data->ssid)) free(data->ssid);
		}
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_wifi_connecting(void *arg){

	WifiCreds_t *creds = (WifiCreds_t *)arg;

	//TODO set animation to main screen wifi symbol

	// create popup with information that wifi is trying to conenct
	UI_WifiScreen_Connecting(creds);

	// free resources
	if(creds){

		if(creds->ssid){
			if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
		}
		if(creds->pass){
			if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
		}
		if(heap_caps_get_allocated_size(creds)) free(creds);
	}
}

static void ui_event_wifi_connect_error(void *arg){

	UI_WifiScreen_ConnectError();

	//TODO delete animation of wifi icon
}

static void ui_event_wifi_get_pass(void *arg){

	if(0 == arg) return;

	WifiCreds_t *creds = (WifiCreds_t *)arg;

	UI_WifiScreen_GetPass(creds);
}

// add single element to list of found wifi networks
static void ui_event_wifilist_add(void *arg){

	if(0 == arg) return;

	UI_BasicAPData_t *data = (UI_BasicAPData_t *)arg;

	UI_WifiScreen_AddToList(data);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_wifilist_clear(void *arg){

	UI_WifiScreen_ClearList();
}

static void ui_event_time_changed(void *arg){

	if(0 == arg) return;

	struct tm *changed_time = (struct tm *)arg;

	UI_MainScreen_UpdateTimeAndDate(changed_time);
}

static void ui_event_clock_not_sync(void *arg){

	UI_MainScreen_SetTopIcon(sync_icon, ICON_NO_SYNC);
}


static void ui_event_clock_sync(void *arg){

	UI_MainScreen_SetTopIcon(sync_icon, ICON_SYNC);

}


static void ui_event_basic_weather_update(void *arg){

	if(0 == arg) return;

	UI_BasicWeatherValues_t *data = (UI_BasicWeatherValues_t *)arg;

	UI_MainScreen_UpdateWeatherIcon(data->icon_path);
	UI_MainScreen_UpdateWeatherData(data->temp, data->press, data->hum);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_detailed_weather_update(void *arg){

	if(0 == arg) return;

	UI_DetailedWeatherValues_t *data = (UI_DetailedWeatherValues_t *)arg;

	detailed_weather_update_city_name(data->city_name);
	detailed_weather_update_country_name(data->country_name);
	detailed_weather_update_weather_icon(data->icon_path);
	detailed_weather_update_avg_temp(data->average_temp);
	detailed_weather_update_min_max_temp(data->min_temp, data->max_temp);
	detailed_weather_update_sunrise_sunset_time(data->sunrise_time, data->sunset_time);
	detailed_weather_update_precip_percent_rain(data->recip_rain, data->percent_rain);
	detailed_weather_update_avg_max_wind(data->wind_avg, data->wind_max);
	detailed_weather_update_precip_percent_snow(data->recip_snow, data->percent_snow);

	_ui_screen_change(&ui_WeatherDetailsScrren, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, ui_WeatherDetailsScrren_screen_init);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_main_scr_wifi_btn_clicked(void *arg){

	UI_WifiScreen_Load(0);
	WIFI_StartScan();
}

static void ui_event_wifi_scr_back_btn_clicked(void *arg){

	UI_MainScreen_Load(0);
}



/*****************************
 * detailed functions
 * ***************************/

static void detailed_weather_update_city_name(char *city_name){

	if(0 == city_name) return;

	lv_label_set_text(ui_WeatherScreenCity, city_name);
	if(heap_caps_get_allocated_size(city_name)) free(city_name);
}

static void detailed_weather_update_country_name(char *country_name){

	if(0 == country_name) return;

	lv_label_set_text(ui_WeatherScreenCountry, country_name);
	if(heap_caps_get_allocated_size(country_name)) free(country_name);
}

static void detailed_weather_update_weather_icon(char *icon_path){

	if(0 == icon_path) return;

	set_weather_icon(icon_path, ui_WeatherScreenIcon);
}

static void detailed_weather_update_avg_temp(int avg_temp){

	uint8_t r, g, b, diff;

	if(((int)-50 > avg_temp) || ((int)50 < avg_temp)) return;

	// set temprature value as text and as arc value
	lv_label_set_text_fmt(ui_WeatherScreenTemp, "%+d°C", avg_temp);
	lv_arc_set_value(ui_WeatherScreenTempArc, avg_temp);

	// calculate color of ui_WeatherScreenTempArc
	/* <------------------------------------------------------------------------------------------------>
	 * -50                                   -10          0          10          25                    50     degC
	 * 0                                      0                      0 --------> 255                   255    R
	 * 0 -----------------------------------> 255                    255         255 <---------------- 0      G
	 * 255                                    255 <----------------- 0           0                     0      B
	 *
	 * The lowest values of temperature are pure blue, rising up, color shifts to green, then yellow, orange and
	 * pure red by highest values
	 * */

	if((int)-10 >= avg_temp){

		r = 0;
		b = 255;
		diff = abs((int)-50 - avg_temp);
		g = ((unsigned int)(diff * 255))/(unsigned int)40;
	}
	else if(((int)-10 < avg_temp) && ((int)10 >= avg_temp)){

		r = 0;
		g = 255;
		diff = abs((int)10 - avg_temp);
		b = ((unsigned int)(diff * 255))/(unsigned int)20;
	}
	else if(((int)10 < avg_temp) && ((int)25 >= avg_temp)){

		g = 255;
		b = 0;
		diff = abs(avg_temp - (int)10);
		r = ((unsigned int)(diff * 255))/(unsigned int)15;
	}
	else{

		r = 255;
		b = 0;
		diff = abs((int)50 - avg_temp);
		g = ((unsigned int)(diff * 255))/(unsigned int)25;
	}

	lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_make(r, g, b), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_make(r, g, b), LV_PART_INDICATOR | LV_STATE_DEFAULT);

}

static void detailed_weather_update_min_max_temp(int min_temp, int max_temp){

	lv_label_set_text_fmt(ui_WeatherScreenTempMinMax, "min/max\n%+d°C / %+d°C", min_temp, max_temp);
}

static void detailed_weather_update_sunrise_sunset_time(char *sunrise_time, char *sunset_time){

	if((0 == sunrise_time) || (0 == sunset_time)) goto cleanup;

	lv_label_set_text_fmt(ui_WeatherScreenSunriseLabel, "%s\n%s", sunrise_time, sunset_time);

	cleanup:
		if(sunrise_time){
			if(heap_caps_get_allocated_size(sunrise_time)) free(sunrise_time);
		}
		if(sunset_time){
			if(heap_caps_get_allocated_size(sunset_time)) free(sunset_time);
		}
}

static void detailed_weather_update_precip_percent_rain(int precip, int percent){

	lv_label_set_text_fmt(ui_WeatherScreenRainLabel, "%dmm\n%d%%", precip, percent);
}

static void detailed_weather_update_avg_max_wind(int avg, int max){

	lv_label_set_text_fmt(ui_WeatherScreenWindLabel, "%dkm/h\n%dkm/h", avg, max);
}

static void detailed_weather_update_precip_percent_snow(int precip, int percent){

	lv_label_set_text_fmt(ui_WeatherScreenSnowLabel, "%dmm\n%d%%", precip, percent);
}

/*****************************
 * helpers
 * ***************************/


/* prepare correct file path to set image from sd card */
static void set_weather_icon(char *path, lv_obj_t * obj){

	size_t len;
	int a;
	char *buff = 0;

	// allocate buffer for image path
	len = strnlen(path, 64);
	if(64 == len) goto cleanup;
	buff = malloc(len + 3);
	if(0 == buff) goto cleanup;

	// prepare path string
	a = sprintf(buff, "%c:%s", LV_FS_STDIO_LETTER, path);
	if(a != (len + 2)) goto cleanup;

	// set image from path
	lv_img_set_src(obj, buff);

	cleanup:
		if(buff){
			if(heap_caps_get_allocated_size(buff)) free(buff);
		}
		if(path){
			if(heap_caps_get_allocated_size(path)) free(path);
		}
}


