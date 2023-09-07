#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "lvgl.h"
#include "ui.h"
#include "ui_styles.h"
#include "ui_main_screen.h"
#include "ui_wifi_screen.h"
#include "ui_weather_screen.h"
#include "ui_task.h"

#include "main.h"
#include "animations.h"
#include "wifi.h"
#include "display.h"
#include "online_requests.h"



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
static void ui_event_weather_scr_back_btn_clicked(void *arg);

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
		[UI_EVT_WEATHERSCR_BACK_BTN_CLICKED] =  ui_event_weather_scr_back_btn_clicked,
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
//	ui_WeatherDetailsScrren_screen_init();
	UI_WeatherScrren_Init();
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

	UI_WeatherScreen_UpdateCityName(data->city_name);
	UI_WeatherScreen_UpdateCountryName(data->country_name);
	UI_WeatherScreen_UpdateWeatherIcon(data->icon_path);
	UI_WeatherScreen_UpdateAvgTemp(data->average_temp);
	UI_WeatherScreen_UpdateMinMaxTemp(data->min_temp, data->max_temp);
	UI_WeatherScreen_UpdateSunriseSunsetTime(data->sunrise_time, data->sunset_time);
	UI_WeatherScreen_UpdatePrecipPercentRain(data->recip_rain, data->percent_rain);
	UI_WeatherScreen_UpdateAvgMaxWind(data->wind_avg, data->wind_max);
	UI_WeatherScreen_UpdatePrecipPercentSnow(data->recip_snow, data->percent_snow);

	UI_WeatherScreen_Load();

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

static void ui_event_weather_scr_back_btn_clicked(void *arg){

	UI_MainScreen_Load(0);
}

