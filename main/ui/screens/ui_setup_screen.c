#include "ui.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_setup_screen_evt_handler(lv_event_t * e);
static void ui_setup_screen_get_themes_list(void);
static void ui_setup_screen_set_active_theme(void);
static void ui_setup_screen_report_selected_theme(void);
static void ui_setup_screen_change_backlight(void);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *ui_SetupScreen, *ui_SetupScreenBackButton,
				*UI_SetupScreenThemePanel, *UI_SetupScreenThemeLabel, *UI_SetupScreenThemeDropdown,
				*UI_SetupScreenBacklightPanel, *UI_SetupScreenBacklightLabel, *UI_SetupScreenBacklightSlider;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize setup screen */
void UI_SetupScreen_Init(void){

	lv_obj_t *dropdown_list;

	UI_ScreenCreate(&ui_SetupScreen);

	UI_BackButtonCreate(&ui_SetupScreen, &ui_SetupScreenBackButton);
	lv_obj_add_event_cb(ui_SetupScreenBackButton, ui_setup_screen_evt_handler, LV_EVENT_RELEASED, NULL);

	UI_SetupScreenThemePanel = lv_obj_create(ui_SetupScreen);
	lv_obj_add_style(UI_SetupScreenThemePanel, &UI_PanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(UI_SetupScreenThemePanel, LV_ALIGN_TOP_MID);
    lv_obj_set_width(UI_SetupScreenThemePanel, 310);
    lv_obj_set_height(UI_SetupScreenThemePanel, 60);
    lv_obj_set_y(UI_SetupScreenThemePanel, 5);
    lv_obj_clear_flag(UI_SetupScreenThemePanel, LV_OBJ_FLAG_SCROLLABLE);

	UI_SetupScreenThemeLabel = lv_label_create(UI_SetupScreenThemePanel);
	lv_obj_add_style(UI_SetupScreenThemeLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(UI_SetupScreenThemeLabel, LV_ALIGN_LEFT_MID);
    lv_obj_set_x(UI_SetupScreenThemeLabel, 15);
    lv_label_set_text(UI_SetupScreenThemeLabel, "Color theme");

    UI_SetupScreenThemeDropdown = lv_dropdown_create(UI_SetupScreenThemePanel);
    lv_obj_add_style(UI_SetupScreenThemeDropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(UI_SetupScreenThemeDropdown, &UI_CheckboxStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(UI_SetupScreenThemeDropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_align(UI_SetupScreenThemeDropdown, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(UI_SetupScreenThemeDropdown, -10);
    lv_obj_set_width(UI_SetupScreenThemeDropdown, 160);
    lv_obj_add_event_cb(UI_SetupScreenThemeDropdown, ui_setup_screen_evt_handler, LV_EVENT_VALUE_CHANGED, NULL);

    dropdown_list = lv_dropdown_get_list(UI_SetupScreenThemeDropdown);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_CHECKED);

    ui_setup_screen_get_themes_list();
    ui_setup_screen_set_active_theme();

    UI_SetupScreenBacklightPanel = lv_obj_create(ui_SetupScreen);
	lv_obj_add_style(UI_SetupScreenBacklightPanel, &UI_PanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(UI_SetupScreenBacklightPanel, LV_ALIGN_TOP_MID);
    lv_obj_set_width(UI_SetupScreenBacklightPanel, 310);
    lv_obj_set_height(UI_SetupScreenBacklightPanel, 60);
    lv_obj_set_y(UI_SetupScreenBacklightPanel, 70);
    lv_obj_clear_flag(UI_SetupScreenBacklightPanel, LV_OBJ_FLAG_SCROLLABLE);

    UI_SetupScreenBacklightLabel = lv_label_create(UI_SetupScreenBacklightPanel);
	lv_obj_add_style(UI_SetupScreenBacklightLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(UI_SetupScreenBacklightLabel, LV_ALIGN_LEFT_MID);
    lv_obj_set_x(UI_SetupScreenBacklightLabel, 15);
    lv_label_set_text(UI_SetupScreenBacklightLabel, "Backlight");

    UI_SetupScreenBacklightSlider = lv_slider_create(UI_SetupScreenBacklightPanel);
    lv_obj_add_style(UI_SetupScreenBacklightSlider, &UI_SliderMainStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(UI_SetupScreenBacklightSlider, &UI_SliderIndicatorStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(UI_SetupScreenBacklightSlider, &UI_SliderKnobStyle, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_align(UI_SetupScreenBacklightSlider, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(UI_SetupScreenBacklightSlider, -10);
    lv_obj_set_width(UI_SetupScreenBacklightSlider, 160);
    lv_obj_set_height(UI_SetupScreenBacklightSlider, 5);
    lv_slider_set_value(UI_SetupScreenBacklightSlider, 50U, LV_ANIM_OFF);
    lv_obj_add_event_cb(UI_SetupScreenBacklightSlider, ui_setup_screen_evt_handler, LV_EVENT_VALUE_CHANGED, NULL);
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
    else if((event_code == LV_EVENT_VALUE_CHANGED) && (target == UI_SetupScreenThemeDropdown)) {

    	ui_setup_screen_report_selected_theme();
    }
    else if((event_code == LV_EVENT_VALUE_CHANGED) && (target == UI_SetupScreenBacklightSlider)) {

    	ui_setup_screen_change_backlight();
    }
}

/* get an array with available theme names and
 * set them as a dropdown options*/
static void ui_setup_screen_get_themes_list(void){

	char buff[512] = {0};
	const char **themes_array;
	uint8_t a = 0, b = 0;

	// get names
    themes_array = UI_GetThemesList(&a);
    if((0 == themes_array) || (0 == a)){

    	lv_dropdown_set_options(UI_SetupScreenThemeDropdown, "error");
    	return;
    }

    // prepre a string with format "themename1\nthemename2\nthemename3
    strcpy(buff, themes_array[b]);
    b++;
    while(b < a){

    	strcat(buff, "\n");
    	strcat(buff, themes_array[b]);
    	b++;
    }

    //set preapred string as options for dropdown list
    lv_dropdown_set_options(UI_SetupScreenThemeDropdown, buff);

    // free the array
    if(heap_caps_get_allocated_size(themes_array)) free(themes_array);
}

/* get the active theme name and set it as selected item in dropdown list */
static void ui_setup_screen_set_active_theme(void){

	const char  *current_theme = 0;
	int32_t idx;

	// get the name
	NVS_GetConfig(CONFIG_THEME, &current_theme);
	if(0 == current_theme) goto error;

	// get an index of this name within dropdown options
	idx = lv_dropdown_get_option_index(UI_SetupScreenThemeDropdown, current_theme);
	if(0 > idx) goto error;

	// set the option with this index as selected
	lv_dropdown_set_selected(UI_SetupScreenThemeDropdown, idx);
	return;

	error:
		lv_dropdown_set_options(UI_SetupScreenThemeDropdown, "error");
}

/* get the new name of selected item and send it to UI task */
static void ui_setup_screen_report_selected_theme(void){

    char buff[32], *return_ptr = 0;
    int a = 0;

    // get the name
	lv_dropdown_get_selected_str(UI_SetupScreenThemeDropdown, buff, sizeof(buff));

	// prepare return data
	a = strnlen(buff, 32);
	if((0 == a) || (32 == a)) return;

	return_ptr = calloc((a + 1), sizeof(char));
	if(0 == return_ptr) return;

	memcpy(return_ptr, buff, (a + 1));

	// report the name to UI task
	UI_ReportEvt(UI_EVT_THEME_CHANGE_REQUEST, return_ptr);
}

/* set the new value of backlight PWM */
static void ui_setup_screen_change_backlight(void){

	uint32_t backlight_value;

	backlight_value = lv_slider_get_value(UI_SetupScreenBacklightSlider);

	Disp_SetBacklight(backlight_value);
}
