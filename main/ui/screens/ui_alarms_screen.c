#include "ui.h"

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
static lv_obj_t *ui_AlarmsScreen, *ui_AlarmsScreenBackButton;


void UI_AlarmsScreen_Init(void){

	UI_ScreenCreate(&ui_AlarmsScreen);

	UI_BackButtonCreate(&ui_AlarmsScreen, &ui_AlarmsScreenBackButton);
	lv_obj_add_event_cb(ui_AlarmsScreenBackButton, ui_alarms_screen_evt_handler, LV_EVENT_RELEASED, NULL);
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
