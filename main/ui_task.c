/**************************************************************
 *	include
 ***************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include <sys/time.h>
#include "esp_log.h"

#include "lvgl.h"
#include "ui.h"

#include "main.h"
#include "animations.h"
#include "wifi.h"
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

	ui_MainScreen_screen_init();

	lv_label_set_text(ui_ClockLabel, "--:--");
	lv_label_set_text(ui_DateLabel, "");
	lv_label_set_text(ui_WiFiIconLabel, "    ");
	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 150, 2000, true);

	while(1){

//		vTaskDelay(pdMS_TO_TICKS(1000));
		ret = xQueueReceive(ui_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			event_tab[data.type](data.arg);
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

	data.type = Type;
	data.arg = arg;

	xQueueSend(ui_queue_handle, &data, pdMS_TO_TICKS(50));
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
 * icon_type is an enum that sets correct position for different icons (0 or 3)
 * icon is a character that represents an icon within font
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

		lv_label_set_text_fmt(ui_DateLabel, "%s\n%02d %s", Eng_DayName[last_displayed_time.tm_wday], last_displayed_time.tm_mday,
				Eng_MonthName_3char[last_displayed_time.tm_mon]);
	}
}

static void ui_event_clock_not_sync(void *arg){

	wifi_label_set(sync_icon, ICON_NO_SYNC);
}


static void ui_event_clock_sync(void *arg){

	wifi_label_set(sync_icon, ICON_SYNC);
}
