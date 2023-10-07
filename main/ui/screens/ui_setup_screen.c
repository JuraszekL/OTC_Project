#include "ui.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_setup_screen_evt_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *ui_SetupScreen, *ui_SetupScreenBackButton;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize setup screen */
void UI_SetupScreen_Init(void){

	UI_ScreenCreate(&ui_SetupScreen);

	UI_BackButtonCreate(&ui_SetupScreen, &ui_SetupScreenBackButton);
	lv_obj_add_event_cb(ui_SetupScreenBackButton, ui_setup_screen_evt_handler, LV_EVENT_RELEASED, NULL);
}

/* load setup screen */
void UI_SetupScreen_Load(void){

	lv_scr_load_anim(ui_SetupScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* setup screen events handler */
static void ui_setup_screen_evt_handler(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if((event_code == LV_EVENT_RELEASED) && (target == ui_SetupScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_SETUPSCR_BACK_BTN_CLICKED, 0);
    }
}
