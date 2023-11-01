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
static void ui_alarm_refresh_panel(uint8_t idx);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
extern const char *Eng_DayName_Short[7];

static lv_obj_t *ui_AlarmsScreen, *ui_AlarmsScreenBackButton;
static struct ui_alarm_object ui_Alarms[ALARMS_NUMBER];

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Initialize alarm screen */
void UI_AlarmsScreen_Init(void){

	uint32_t a;

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

	    ui_alarm_refresh_panel(a);

	    lv_obj_add_event_cb(ui_Alarms[a].ui_AlarmScreenPanel, ui_alarms_screen_evt_handler, LV_EVENT_RELEASED, (void *)a);
	    lv_obj_add_event_cb(ui_Alarms[a].ui_AlarmScreenSwitch, ui_alarms_screen_evt_handler, LV_EVENT_VALUE_CHANGED, (void *)a);
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
    uint32_t user_data;

    if((event_code == LV_EVENT_RELEASED) && (target == ui_AlarmsScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_SETUPSCR_BACK_BTN_CLICKED, 0);
    }
    else if(event_code == LV_EVENT_VALUE_CHANGED) {

    	user_data = (uint32_t)lv_event_get_user_data(e);
    	if(user_data >= ALARMS_NUMBER) return;

    	if(target == ui_Alarms[user_data].ui_AlarmScreenSwitch){

    		//UI_ReportEvt(UI_EVT_ALARM_SWICH_CHANGED, (void *)user_data);
    	}
    }
    else if(event_code == LV_EVENT_RELEASED) {

    	user_data = (uint32_t)lv_event_get_user_data(e);
    	if(user_data >= ALARMS_NUMBER) return;

    	if(target == ui_Alarms[user_data].ui_AlarmScreenPanel){

    		//UI_ReportEvt(UI_EVT_ALARM_PANEL_CLICKED, (void *)user_data);
    	}
    }
}

/* refresh values displayed on alarm's panel */
static void ui_alarm_refresh_panel(uint8_t idx){

	AlarmData_t *alarm;
	char buff[64] = {0};
	uint8_t a;

	// get current values from RAM array
	alarm = Alarm_GetCurrentValues(idx);
	if(0 == alarm) goto error;

	// set the time of alarm
	lv_label_set_text_fmt(ui_Alarms[idx].ui_AlarmScreenHourLabel, "%02d:%02d", alarm->hour, alarm->minute);

	// check the flags for every single day in a week
	// bits are organised in the same way that POSIX "struct tm"
	// if bit is set then add the name of the day to the label buffer (separated by a space character)
	for(a = 0; a < 7; a++){

		if(alarm->flags & (1 << a)){

			strcat(buff, Eng_DayName_Short[a]);
			strcat(buff, " ");
		}
	}

	lv_label_set_text(ui_Alarms[idx].ui_AlarmScreenDaysLabel, buff);

	// set the switch state
	if(true == alarm->status) lv_obj_add_state(ui_Alarms[idx].ui_AlarmScreenSwitch, LV_STATE_CHECKED);
	else lv_obj_clear_state(ui_Alarms[idx].ui_AlarmScreenSwitch, LV_STATE_CHECKED);

	if(alarm->text){
		if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
	}
	free(alarm);
	return;

	error:
		lv_label_set_text(ui_Alarms[idx].ui_AlarmScreenDaysLabel, "");
		lv_label_set_text(ui_Alarms[idx].ui_AlarmScreenHourLabel, "");
		lv_obj_add_state(ui_Alarms[idx].ui_AlarmScreenSwitch, LV_STATE_DISABLED);
		lv_obj_add_flag(ui_Alarms[idx].ui_AlarmScreenSwitch, LV_OBJ_FLAG_HIDDEN);
		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}
}
