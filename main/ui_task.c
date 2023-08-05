/**************************************************************
 *	include
 ***************************************************************/
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
static void ui_event_time_changed(void *arg);
static void ui_event_clock_not_sync(void *arg);
static void ui_event_clock_sync(void *arg);
static void ui_event_basic_weather_icon_update(void *arg);
static void ui_event_basic_weather_values_update(void *arg);
static void ui_event_weather_city_update(void *arg);
static void ui_event_weather_country_update(void *arg);
static void ui_event_weather_icon_update(void *arg);
static void ui_event_weather_avg_temp_update(void *arg);

static void set_weater_icon(char *path, lv_obj_t * obj);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
const char version[] = {VERSION};
const ui_event event_tab[] = {

		[UI_EVT_WIFI_CONNECTED] = ui_event_wifi_connected,
		[UI_EVT_WIFI_DISCONNECTED] = ui_event_wifi_disconnected,
		[UI_EVT_TIME_CHANGED] = ui_event_time_changed,
		[UI_EVT_CLOCK_NOT_SYNC] = ui_event_clock_not_sync,
		[UI_EVT_CLOCK_SYNC] = ui_event_clock_sync,
		[UI_EVT_BASIC_WEATHER_ICON_UPDATE] = ui_event_basic_weather_icon_update,
		[UI_EVT_BASIC_WEATHER_VALUES_UPDATE] = ui_event_basic_weather_values_update,
		[UI_EVT_WEATHER_CITY_UPDATE] = ui_event_weather_city_update,
		[UI_EVT_WEATHER_COUNTRY_UPDATE] = ui_event_weather_country_update,
		[UI_EVT_WEATHER_ICON_UPDATE] = ui_event_weather_icon_update,
		[UI_EVT_WEATHER_AVG_TEMP_UPDATE] = ui_event_weather_avg_temp_update,
};

extern const char *Eng_DayName[7];
extern const char *Eng_MonthName_3char[12];

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

	// notify wifi task to start wifi initialization when nothing happens on the screen
	xTaskNotifyGive(Wifi_TaskHandle);

	xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
	ui_MainScreen_screen_init();
	ui_WeatherDetailsScrren_screen_init();

	lv_label_set_text(ui_ClockLabel, "--:--");
	lv_label_set_text(ui_DateLabel, "");
	lv_label_set_text(ui_WiFiIconLabel, "    ");
	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 150, 2000, true);
	xSemaphoreGive(LVGL_MutexHandle);

	while(1){

//		vTaskDelay(pdMS_TO_TICKS(1000));
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

	res = xQueueSend(ui_queue_handle, &data, pdMS_TO_TICKS(50));
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

/* set icon within area of ui_WiFiIconLabel
 *
 * format of ui_WiFiIconLabel is char-space-space-char-null, each one charactes is a single icon,
 * correct string should contain 4 characters and null character at the end
 * par. icon_type is an enum that sets correct position for different icons (0 or 3)
 * par. icon is a character that represents an icon within font
 * */
static void wifi_label_set(uint8_t icon_type, char icon){

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

static void ui_event_wifi_disconnected(void *arg){

	wifi_label_set(wifi_icon, ICON_NO_WIFI);
}

static void ui_event_wifi_connected(void *arg){

	wifi_label_set(wifi_icon, ICON_WIFI);
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

	wifi_label_set(sync_icon, ICON_NO_SYNC);
}


static void ui_event_clock_sync(void *arg){

	wifi_label_set(sync_icon, ICON_SYNC);

}

static void set_weater_icon(char *path, lv_obj_t * obj){

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

/* function sets weather basic data on the main screen */
static void ui_event_basic_weather_icon_update(void *arg){

	if(0 == arg) return;

	set_weater_icon((char *)arg, ui_WeatherImage);
}

static void ui_event_basic_weather_values_update(void *arg){

	if(0 == arg) return;

	UI_WeatherValues_t *ptr = (UI_WeatherValues_t *)arg;

	lv_label_set_text_fmt(ui_WeatherLabel, "%d°C\n%dhPa\n%d%%", ptr->temp, ptr->press, ptr->hum);

	if(ptr){
		if(heap_caps_get_allocated_size(ptr)) free(ptr);
	}
}

static void ui_event_weather_city_update(void *arg){

	if(arg){

		lv_label_set_text(ui_WeatherScreenCity, (char *)arg);
		if(heap_caps_get_allocated_size(arg)) free(arg);
	}
}

static void ui_event_weather_country_update(void *arg){

	if(arg){

		lv_label_set_text(ui_WeatherScreenCountry, (char *)arg);
		if(heap_caps_get_allocated_size(arg)) free(arg);
	}
}

static void ui_event_weather_icon_update(void *arg){

	if(0 == arg) return;

	set_weater_icon((char *)arg, ui_WeatherScreenIcon);
}

static void ui_event_weather_avg_temp_update(void *arg){

	int avg_temp = (int)arg;
	char buff[6];
	uint8_t r, g, b, diff;

	if(((int)-50 > avg_temp) || ((int)50 < avg_temp)) return;

	sprintf(buff, "%+d°C", avg_temp);
	lv_label_set_text(ui_WeatherScreenTemp, buff);
	lv_arc_set_value(ui_WeatherScreenTempArc, avg_temp);

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

void WetaherScreen_BackButtonClicked(lv_event_t * e){

//	ESP_LOGI("", "clicked");
}

void MainScreen_WeatherIconClicked(lv_event_t * e){

	_ui_screen_change(&ui_WeatherDetailsScrren, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, ui_WeatherDetailsScrren_screen_init);
	OnlineRequest_Send(ONLINEREQ_DETAILED_UPDATE, NULL);
}
