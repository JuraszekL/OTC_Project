#include "ui.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
struct ui_alarm_object {

	lv_obj_t *ui_AlarmScreenPanel;
	lv_obj_t *ui_AlarmScreenHourLabel;
	lv_obj_t *ui_AlarmScreenDaysLabel;
	lv_obj_t *ui_AlarmScreenSwitch;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_alarms_screen_evt_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
extern const char *Eng_DayName_Short[7];

static lv_obj_t *ui_AlarmsScreen, *ui_AlarmsScreenBackButton;
static struct ui_alarm_object ui_Alarms[ALARMS_NUMBER];


void UI_AlarmsScreen_Init(void){

	uint8_t a;

	UI_ScreenCreate(&ui_AlarmsScreen);

	UI_BackButtonCreate(&ui_AlarmsScreen, &ui_AlarmsScreenBackButton);
	lv_obj_add_event_cb(ui_AlarmsScreenBackButton, ui_alarms_screen_evt_handler, LV_EVENT_RELEASED, NULL);

	for(a = 0; a < ALARMS_NUMBER; a++){

		ui_Alarms[a].ui_AlarmScreenPanel = lv_obj_create(ui_AlarmsScreen);
		lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenPanel, &UI_PanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_align(ui_Alarms[a].ui_AlarmScreenPanel, LV_ALIGN_TOP_MID);
	    lv_obj_set_width(ui_Alarms[a].ui_AlarmScreenPanel, 310);
	    lv_obj_set_height(ui_Alarms[a].ui_AlarmScreenPanel, 90);
	    lv_obj_set_y(ui_Alarms[a].ui_AlarmScreenPanel, ((a * 95U) + 5U));
	    lv_obj_clear_flag(ui_Alarms[a].ui_AlarmScreenPanel, LV_OBJ_FLAG_SCROLLABLE);

	    ui_Alarms[a].ui_AlarmScreenHourLabel = lv_label_create(ui_Alarms[a].ui_AlarmScreenPanel);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenHourLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_width(ui_Alarms[a].ui_AlarmScreenHourLabel, LV_SIZE_CONTENT);
	    lv_obj_set_height(ui_Alarms[a].ui_AlarmScreenHourLabel, LV_SIZE_CONTENT);
	    lv_obj_set_x(ui_Alarms[a].ui_AlarmScreenHourLabel, 10);
	    lv_obj_set_y(ui_Alarms[a].ui_AlarmScreenHourLabel, 10);
	    lv_obj_set_align(ui_Alarms[a].ui_AlarmScreenHourLabel, LV_ALIGN_TOP_LEFT);
	    lv_label_set_text(ui_Alarms[a].ui_AlarmScreenHourLabel, "");

	    ui_Alarms[a].ui_AlarmScreenDaysLabel = lv_label_create(ui_Alarms[a].ui_AlarmScreenPanel);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenDaysLabel, &UI_Text14Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_width(ui_Alarms[a].ui_AlarmScreenDaysLabel, LV_SIZE_CONTENT);
	    lv_obj_set_height(ui_Alarms[a].ui_AlarmScreenDaysLabel, LV_SIZE_CONTENT);
	    lv_obj_set_x(ui_Alarms[a].ui_AlarmScreenDaysLabel, 10);
	    lv_obj_set_y(ui_Alarms[a].ui_AlarmScreenDaysLabel, -10);
	    lv_obj_set_align(ui_Alarms[a].ui_AlarmScreenDaysLabel, LV_ALIGN_BOTTOM_LEFT);
	    lv_label_set_text(ui_Alarms[a].ui_AlarmScreenDaysLabel, "");

	    ui_Alarms[a].ui_AlarmScreenSwitch = lv_switch_create(ui_Alarms[a].ui_AlarmScreenPanel);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenSwitch, &UI_SwitchMainStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenSwitch, &UI_SwitchIndicatorDefaultStyle, LV_PART_INDICATOR |
	    		LV_STATE_DEFAULT);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenSwitch, &UI_SwitchIndicatorCheckedStyle, LV_PART_INDICATOR |
	    		LV_STATE_CHECKED);
	    lv_obj_add_style(ui_Alarms[a].ui_AlarmScreenSwitch, &UI_SwitchKnobStyle, LV_PART_KNOB | LV_STATE_DEFAULT);
	    lv_obj_set_width(ui_Alarms[a].ui_AlarmScreenSwitch, 50);
	    lv_obj_set_height(ui_Alarms[a].ui_AlarmScreenSwitch, 25);
	    lv_obj_set_x(ui_Alarms[a].ui_AlarmScreenSwitch, -15);
	    lv_obj_set_y(ui_Alarms[a].ui_AlarmScreenSwitch, 15);
	    lv_obj_set_align(ui_Alarms[a].ui_AlarmScreenSwitch, LV_ALIGN_TOP_RIGHT);
	}
}

void UI_AlarmsScreen_Load(void){

	lv_scr_load_anim(ui_AlarmsScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* setup screen events handler */
static void ui_alarms_screen_evt_handler(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if((event_code == LV_EVENT_RELEASED) && (target == ui_AlarmsScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_SETUPSCR_BACK_BTN_CLICKED, 0);
    }
//    else if((event_code == LV_EVENT_VALUE_CHANGED) && (target == UI_SetupScreenThemeDropdown)) {
//
//    	ui_setup_screen_report_selected_theme();
//    }
//    else if((event_code == LV_EVENT_VALUE_CHANGED) && (target == UI_SetupScreenBacklightSlider)) {
//
//    	ui_setup_screen_change_backlight();
//    }
}
