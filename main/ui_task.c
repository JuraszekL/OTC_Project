#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <sys/time.h>
#include "esp_log.h"

#include "lvgl.h"
#include "lv_conf.h"
#include "ui.h"

#include "main.h"
#include "animations.h"
#include "wifi.h"
#include "display.h"
#include "online_requests.h"
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

enum {wifi_icon = 0, sync_icon = 3};

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
static void ui_event_wifilist_add(void *arg);
static void ui_event_wifilist_clear(void *arg);
static void ui_event_time_changed(void *arg);
static void ui_event_clock_not_sync(void *arg);
static void ui_event_clock_sync(void *arg);
static void ui_event_basic_weather_update(void *arg);
static void ui_event_detailed_weather_update(void *arg);
static void ui_event_main_scr_wifi_btn_clicked(void *arg);
static void ui_event_wifi_scr_back_btn_clicked(void *arg);


static void basic_weather_update_icon(char *icon_path);
static void basic_weather_update_values(int temp, int press, int hum);

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
static void set_wifi_label(uint8_t icon_type, char icon);
static void set_ap_details(UI_DetailedAPData_t *data);

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

extern const char *Eng_DayName[7];
extern const char *Eng_MonthName_3char[12];
extern const char *Authentication_Modes[];

static QueueHandle_t ui_queue_handle;
static struct {

	uint8_t	tm_min;
	uint8_t	tm_hour;
	uint8_t	tm_mday;
	uint8_t	tm_mon;
	uint8_t	tm_wday;

} last_displayed_time;

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

	// run startup screen
	startup_screen();

	xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
	ui_MainScreen_screen_init();
	ui_WeatherDetailsScrren_screen_init();
	ui_WifiScreen_screen_init();

	lv_label_set_text(ui_ClockLabel, "--:--");
	lv_label_set_text(ui_DateLabel, "");
	lv_label_set_text(ui_WiFiIconLabel, "    ");
	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 150, 2000, true);
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

//	ESP_LOGI("", "clicked");
}

void MainScreen_WifiButtonClicked(lv_event_t * e){

	UI_ReportEvt(UI_EVT_MAINSCR_WIFI_BTN_CLICKED, 0);
}

void WifiScreenBackButtonClicked(lv_event_t * e){

	UI_ReportEvt(UI_EVT_WIFISCR_BACK_BTN_CLICKED, 0);
}

void MainScreen_WeatherIconClicked(lv_event_t * e){

	OnlineRequest_Send(ONLINEREQ_DETAILED_UPDATE, NULL);
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
	set_wifi_label(wifi_icon, ICON_NO_WIFI);

	// clear information about connected AP
	set_ap_details(0);
}

static void ui_event_wifi_connected(void *arg){

	UI_DetailedAPData_t *data = (UI_DetailedAPData_t *)arg;

	// set wifi icon in top left corner of main screen
	set_wifi_label(wifi_icon, ICON_WIFI);

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		if(0 != data){

			UI_WifiPopup_Connected(data->ssid);
		}
		else{

			UI_WifiPopup_Connected(0);
		}
	}

	// set information about connected AP
	set_ap_details(data);
}

static void ui_event_wifi_connecting(void *arg){

	if(0 == arg) return;

	WifiCreds_t *creds = (WifiCreds_t *)arg;

	// set animation to main screen wifi symbol

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_Connecting(creds->ssid);
	}

	// free resources
	if(creds->ssid){
		if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
	}
	if(creds->pass){
		if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
	}
	if(creds){
		if(heap_caps_get_allocated_size(creds)) free(creds);
	}
}

static void ui_event_wifi_connect_error(void *arg){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_NotConnected();
	}

	// delete animation of wifi icon
}

// add single element to list of found wifi networks
static void ui_event_wifilist_add(void *arg){

	if(0 == arg) return;

	UI_BasicAPData_t *data = (UI_BasicAPData_t *)arg;

	UI_WifiListAdd(data->is_protected, data->ssid, data->rssi);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_wifilist_clear(void *arg){

	UI_WifiListClear();
}

static void ui_event_time_changed(void *arg){

	struct tm *changed_time = (struct tm *)arg;
	uint8_t set_time = 0, set_date = 0;

	// check for differences
	if(last_displayed_time.tm_min != changed_time->tm_min){

		set_time = 1;
		last_displayed_time.tm_min = changed_time->tm_min;
	}

	if(last_displayed_time.tm_hour != changed_time->tm_hour){

		set_time = 1;
		last_displayed_time.tm_hour = changed_time->tm_hour;
	}

	if(last_displayed_time.tm_wday != changed_time->tm_wday){

		set_date = 1;
		last_displayed_time.tm_wday = changed_time->tm_wday;
	}

	if(last_displayed_time.tm_mday != changed_time->tm_mday){

		set_date = 1;
		last_displayed_time.tm_mday = changed_time->tm_mday;
	}

	if(last_displayed_time.tm_mon != changed_time->tm_mon){

		set_date = 1;
		last_displayed_time.tm_mon = changed_time->tm_mon;
	}

	// set required values to lvgl labels
	if(1 == set_time){

		lv_label_set_text_fmt(ui_ClockLabel, "%02d:%02d", last_displayed_time.tm_hour, last_displayed_time.tm_min);
	}

	if(1 == set_date){

		lv_label_set_text_fmt(ui_DateLabel, "%s, %02d %s", Eng_DayName[last_displayed_time.tm_wday], last_displayed_time.tm_mday,
				Eng_MonthName_3char[last_displayed_time.tm_mon]);
	}
}

static void ui_event_clock_not_sync(void *arg){

	set_wifi_label(sync_icon, ICON_NO_SYNC);
}


static void ui_event_clock_sync(void *arg){

	set_wifi_label(sync_icon, ICON_SYNC);

}


static void ui_event_basic_weather_update(void *arg){

	if(0 == arg) return;

	UI_BasicWeatherValues_t *data = (UI_BasicWeatherValues_t *)arg;

	basic_weather_update_icon(data->icon_path);
	basic_weather_update_values(data->temp, data->press, data->hum);

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

	if(false == lv_obj_is_valid(ui_WifiScreen)) ui_WifiScreen_screen_init();
	//add style to inactive state
	UI_WifiListInit();
	lv_scr_load_anim(ui_WifiScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
	WIFI_StartScan();
}

static void ui_event_wifi_scr_back_btn_clicked(void *arg){

	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}



/*****************************
 * detailed functions
 * ***************************/

/* function sets weather basic data on the main screen */
static void basic_weather_update_icon(char *icon_path){

	if(0 == icon_path) return;

	set_weather_icon(icon_path, ui_WeatherImage);
}

static void basic_weather_update_values(int temp, int press, int hum){

	lv_label_set_text_fmt(ui_WeatherLabel, "%d°C\n%dhPa\n%d%%", temp, press, hum);

}

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


/* set icon within area of ui_WiFiIconLabel
 *
 * format of ui_WiFiIconLabel is char-space-space-char-null, each one charactes is a single icon,
 * correct string should contain 4 characters and null character at the end
 * par. icon_type is an enum that sets correct position for different icons (0 or 3)
 * par. icon is a character that represents an icon within font
 * */
static void set_wifi_label(uint8_t icon_type, char icon){

	size_t len = 0;
	char buff[5] = {0};
	char *text = lv_label_get_text(ui_WiFiIconLabel);
	if(0 != text){

		len = strnlen(text, 5);
		if(4 == len){

			// if correct format "%c  %c\0"
			if(text[icon_type] == icon) return;
			else{

				// set correct icon to correct position
				text[icon_type] = icon;
				lv_label_set_text(ui_WiFiIconLabel, text);
			}
		}
		else{

			sprintf(buff, "    ");
			buff[icon_type] = icon;
			lv_label_set_text(ui_WiFiIconLabel, buff);
		}
	}
}


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

/* set/clear details about connected AP on WifiScreen */
static void set_ap_details(UI_DetailedAPData_t *data){

	const char *mode_str = 0;

	if(0 != data){

		// get pointer to authentication mode string
		mode_str = Authentication_Modes[data->mode];

		lv_arc_set_value(ui_WifiScreenRSSIArc, data->rssi);
		lv_label_set_text_fmt(ui_WifiScreenRSSIValueLabel, "%d", data->rssi);
		lv_label_set_text(ui_WifiScreenSSIDLabel, data->ssid);
		lv_label_set_text_fmt(ui_WifiScreenWifiDetails, "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n"
				"IPv4: %s\n%s", data->mac[0], data->mac[1], data->mac[2], data->mac[3], data->mac[4],
				data->mac[5], (char *)data->ip, mode_str);

		free(data->ssid);
		free(data);
	}
	else{

		lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
		lv_label_set_text(ui_WifiScreenRSSIValueLabel, "---");
		lv_label_set_text(ui_WifiScreenSSIDLabel, "");
		lv_label_set_text(ui_WifiScreenWifiDetails, "");
	}
}
