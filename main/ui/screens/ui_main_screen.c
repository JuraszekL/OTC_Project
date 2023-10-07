#include "ui.h"

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
static lv_obj_t *ui_MainScreen, *ui_MainScreenClockLabel, *ui_MainScreenDateLabel, *ui_MainScreenTopIconsLabel,
			*ui_MainScreenTopPanel, *ui_MainScreenWeatherIcon, *ui_MainScreenWeatherLabel, *ui_MainScreenWifiButton,
			*ui_MainScreenSetupButton;

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

	UI_ScreenCreate(&ui_MainScreen);

    ui_MainScreenClockLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_MainScreenClockLabel, &UI_ClockStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_MainScreenClockLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_MainScreenClockLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_MainScreenClockLabel, 0);
    lv_obj_set_y(ui_MainScreenClockLabel, 60);
    lv_obj_set_align(ui_MainScreenClockLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_MainScreenClockLabel, "--:--");

    ui_MainScreenDateLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_MainScreenDateLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_MainScreenDateLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_MainScreenDateLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_MainScreenDateLabel, 0);
    lv_obj_set_y(ui_MainScreenDateLabel, -30);
    lv_obj_set_align(ui_MainScreenDateLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_MainScreenDateLabel, "");

	ui_MainScreenTopPanel = lv_obj_create(ui_MainScreen);
	lv_obj_add_style(ui_MainScreenTopPanel, &UI_PanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(ui_MainScreenTopPanel, LV_ALIGN_TOP_MID);
    lv_obj_set_width(ui_MainScreenTopPanel, 310);
    lv_obj_set_height(ui_MainScreenTopPanel, 50);
    lv_obj_set_y(ui_MainScreenTopPanel, 5);
    lv_obj_clear_flag(ui_MainScreenTopPanel, LV_OBJ_FLAG_SCROLLABLE);

    ui_MainScreenTopIconsLabel = lv_label_create(ui_MainScreenTopPanel);
    lv_obj_add_style(ui_MainScreenTopIconsLabel, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_MainScreenTopIconsLabel, LV_ALIGN_LEFT_MID);
    lv_obj_set_width(ui_MainScreenTopIconsLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_MainScreenTopIconsLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_MainScreenTopIconsLabel, 4);
    lv_label_set_text(ui_MainScreenTopIconsLabel, "    ");

    ui_MainScreenWeatherIcon = lv_img_create(ui_MainScreen);
    lv_obj_set_width(ui_MainScreenWeatherIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_MainScreenWeatherIcon, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_MainScreenWeatherIcon, 55);
    lv_obj_set_y(ui_MainScreenWeatherIcon, 80);
    lv_obj_set_align(ui_MainScreenWeatherIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_MainScreenWeatherIcon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_MainScreenWeatherIcon, LV_OBJ_FLAG_SCROLLABLE);
    lv_img_set_zoom(ui_MainScreenWeatherIcon, 512);
    lv_obj_add_event_cb(ui_MainScreenWeatherIcon, ui_main_screen_evt_handler, LV_EVENT_RELEASED, NULL);

    ui_MainScreenWeatherLabel = lv_label_create(ui_MainScreen);
    lv_obj_add_style(ui_MainScreenWeatherLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_MainScreenWeatherLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_MainScreenWeatherLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_MainScreenWeatherLabel, 160);
    lv_obj_set_y(ui_MainScreenWeatherLabel, 80);
    lv_obj_set_align(ui_MainScreenWeatherLabel, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_align(ui_MainScreenWeatherLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_MainScreenWeatherLabel, "");

    UI_ButtonCreate(&ui_MainScreen, &ui_MainScreenWifiButton, ICON_WIFI);
    lv_obj_set_x(ui_MainScreenWifiButton, 20);
    lv_obj_set_y(ui_MainScreenWifiButton, -20);
    lv_obj_set_align(ui_MainScreenWifiButton, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_event_cb(ui_MainScreenWifiButton, ui_main_screen_evt_handler, LV_EVENT_RELEASED, NULL);

    UI_ButtonCreate(&ui_MainScreen, &ui_MainScreenSetupButton, ICON_SETUP);
    lv_obj_set_x(ui_MainScreenSetupButton, -20);
    lv_obj_set_y(ui_MainScreenSetupButton, -20);
    lv_obj_set_align(ui_MainScreenSetupButton, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_event_cb(ui_MainScreenSetupButton, ui_main_screen_evt_handler, LV_EVENT_RELEASED, NULL);
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
	lv_img_set_src(ui_MainScreenWeatherIcon, buff);

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

	lv_label_set_text_fmt(ui_MainScreenWeatherLabel, "%d°C\n%dhPa\n%d%%", temp, press, hum);

}

/* set icon within area of ui_MainScreenTopIconsLabel
 *
 * format of ui_MainScreenTopIconsLabel is char-space-space-char-null, each one charactes is a single icon,
 * correct string should contain 4 characters and null character at the end
 * par. type is an enum that sets correct position for different icons (0 or 3)
 * par. icon is a character that represents an icon within font
 * */
void UI_MainScreen_SetTopIcon(UI_TopIconType_t type, char icon){

	size_t len = 0;
	char buff[5] = {0};
	char *text = lv_label_get_text(ui_MainScreenTopIconsLabel);
	if(0 != text){

		len = strnlen(text, 5);
		if(4 == len){

			// if correct format "%c  %c\0"
			if(text[type] == icon) return;
			else{

				// set correct icon to correct position
				text[type] = icon;
				lv_label_set_text(ui_MainScreenTopIconsLabel, text);
			}
		}
		else{

			sprintf(buff, "    ");
			buff[type] = icon;
			lv_label_set_text(ui_MainScreenTopIconsLabel, buff);
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

			lv_label_set_text_fmt(ui_MainScreenClockLabel, "%02d:%02d", last_displayed_time.tm_hour, last_displayed_time.tm_min);
		}

		if(1 == set_date){

			lv_label_set_text_fmt(ui_MainScreenDateLabel, "%s, %02d %s", Eng_DayName[last_displayed_time.tm_wday],
					last_displayed_time.tm_mday, Eng_MonthName_3char[last_displayed_time.tm_mon]);
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

    if((event_code == LV_EVENT_RELEASED) && (target == ui_MainScreenWeatherIcon)) {

    	OnlineRequest_Send(ONLINEREQ_DETAILED_UPDATE, NULL);
    }
    else if((event_code == LV_EVENT_RELEASED) && (target == ui_MainScreenWifiButton)) {

    	UI_ReportEvt(UI_EVT_MAINSCR_WIFI_BTN_CLICKED, 0);
    }
    else if((event_code == LV_EVENT_RELEASED) && (target == ui_MainScreenSetupButton)) {

    	UI_ReportEvt(UI_EVT_MAINSCR_SETUP_BTN_CLICKED, 0);
    }
}
