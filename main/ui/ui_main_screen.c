#include "ui_main_screen.h"
#include "ui_styles.h"
#include "ui.h"
#include "ui_task.h"
#include "online_requests.h"
#include "lvgl.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_main_screen_evt_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *ui_MainScreen, *ui_ClockLabel, *ui_DateLabel, *ui_TopIconsLabel, *ui_WhiteLineTop, *ui_WeatherIcon,
			*ui_WeatherLabel, *ui_WifiButton;

static struct {

	uint8_t	tm_min;
	uint8_t	tm_hour;
	uint8_t	tm_mday;
	uint8_t	tm_mon;
	uint8_t	tm_wday;

} last_displayed_time;

extern const char *Eng_DayName[7];
extern const char *Eng_MonthName_3char[12];


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Initialize main screen */
void UI_MainScreen_Init(void){

	lv_obj_t *wifi_but_label;

    ui_MainScreen = lv_obj_create(NULL);
    lv_obj_add_style(ui_MainScreen, &UI_ScreenStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_MainScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_ClockLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_ClockLabel, &UI_ClockLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_ClockLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ClockLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_ClockLabel, 0);
    lv_obj_set_y(ui_ClockLabel, 60);
    lv_obj_set_align(ui_ClockLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_ClockLabel, "--:--");

    ui_DateLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_DateLabel, &UI_Label30ContrastStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_DateLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_DateLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_DateLabel, 0);
    lv_obj_set_y(ui_DateLabel, -30);
    lv_obj_set_align(ui_DateLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_DateLabel, "");

    ui_TopIconsLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_TopIconsLabel, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_TopIconsLabel, LV_ALIGN_TOP_LEFT);
    lv_obj_set_width(ui_TopIconsLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_TopIconsLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_TopIconsLabel, 4);
    lv_obj_set_y(ui_TopIconsLabel, 4);
    lv_label_set_text(ui_TopIconsLabel, "    ");

    //TODO ogarnąć!
    ui_WhiteLineTop = lv_obj_create(ui_MainScreen);
    lv_obj_set_width(ui_WhiteLineTop, 320);
    lv_obj_set_height(ui_WhiteLineTop, 2);
    lv_obj_set_x(ui_WhiteLineTop, 0);
    lv_obj_set_y(ui_WhiteLineTop, 45);
    lv_obj_set_align(ui_WhiteLineTop, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_WhiteLineTop, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_WeatherIcon = lv_img_create(ui_MainScreen);
    lv_obj_set_width(ui_WeatherIcon, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WeatherIcon, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WeatherIcon, 55);
    lv_obj_set_y(ui_WeatherIcon, 100);
    lv_obj_set_align(ui_WeatherIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_WeatherIcon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_WeatherIcon, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_img_set_zoom(ui_WeatherIcon, 512);
    lv_obj_add_event_cb(ui_WeatherIcon, ui_main_screen_evt_handler, LV_EVENT_ALL, NULL);

    ui_WeatherLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_WeatherLabel, &UI_Label30DarkStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WeatherLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WeatherLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WeatherLabel, 160);
    lv_obj_set_y(ui_WeatherLabel, 100);
    lv_obj_set_align(ui_WeatherLabel, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_align(ui_WeatherLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_WeatherLabel, "");

    ui_WifiButton = lv_btn_create(ui_MainScreen);
    lv_obj_add_style(ui_WifiButton, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WifiButton, 20);
    lv_obj_set_y(ui_WifiButton, -20);
    lv_obj_set_align(ui_WifiButton, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_event_cb(ui_WifiButton, ui_main_screen_evt_handler, LV_EVENT_ALL, NULL);

    wifi_but_label = lv_label_create(ui_WifiButton);
    lv_obj_add_style(wifi_but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(wifi_but_label, "%c", ICON_WIFI);
}

/* load main screen with selected delay */
void UI_MainScreen_Load(uint32_t delay){

	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, delay, false);
}

/* set weather icon on the main screen */
void UI_MainScreen_UpdateWeatherIcon(char *icon_path){

	if(0 == icon_path) return;

	size_t len;
	int a;
	char *buff = 0;

	// allocate buffer for image path
	len = strnlen(icon_path, 64);
	if(64 == len) goto cleanup;
	buff = malloc(len + 3);
	if(0 == buff) goto cleanup;

	// prepare path string
	a = sprintf(buff, "%c:%s", LV_FS_STDIO_LETTER, icon_path);
	if(a != (len + 2)) goto cleanup;

	// set image from path
	lv_img_set_src(ui_WeatherIcon, buff);

	cleanup:
		if(buff){
			if(heap_caps_get_allocated_size(buff)) free(buff);
		}
		if(icon_path){
			if(heap_caps_get_allocated_size(icon_path)) free(icon_path);
		}

}

/* set weather basic data on the main screen */
void UI_MainScreen_UpdateWeatherData(int temp, int press, int hum){

	lv_label_set_text_fmt(ui_WeatherLabel, "%d°C\n%dhPa\n%d%%", temp, press, hum);

}

/* set icon within area of ui_TopIconsLabel
 *
 * format of ui_TopIconsLabel is char-space-space-char-null, each one charactes is a single icon,
 * correct string should contain 4 characters and null character at the end
 * par. type is an enum that sets correct position for different icons (0 or 3)
 * par. icon is a character that represents an icon within font
 * */
void UI_MainScreen_SetTopIcon(UI_TopIconType_t type, char icon){

	size_t len = 0;
	char buff[5] = {0};
	char *text = lv_label_get_text(ui_TopIconsLabel);
	if(0 != text){

		len = strnlen(text, 5);
		if(4 == len){

			// if correct format "%c  %c\0"
			if(text[type] == icon) return;
			else{

				// set correct icon to correct position
				text[type] = icon;
				lv_label_set_text(ui_TopIconsLabel, text);
			}
		}
		else{

			sprintf(buff, "    ");
			buff[type] = icon;
			lv_label_set_text(ui_TopIconsLabel, buff);
		}
	}
}

/* set date and time on the main screen */
void UI_MainScreen_UpdateTimeAndDate(struct tm *changed_time){

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

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* main screen events handler */
static void ui_main_screen_evt_handler(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if((event_code == LV_EVENT_CLICKED) && (target == ui_WeatherIcon)) {

    	OnlineRequest_Send(ONLINEREQ_DETAILED_UPDATE, NULL);
    }
    else if((event_code == LV_EVENT_CLICKED) && (target == ui_WifiButton)) {

    	UI_ReportEvt(UI_EVT_MAINSCR_WIFI_BTN_CLICKED, 0);
    }
}
