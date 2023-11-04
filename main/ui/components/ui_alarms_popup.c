#include "ui.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define ALARMS_POPUP_HEIGH_FULL					280
#define ALARMS_POPUP_HEIGH_FOLDED				240

#define ALARMS_POPUP_MUTEX_TIMEOUT_MS			100U

enum { show_keyboard, hide_keyboard };
enum { show_checkboxes, hide_checkboxes };

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void alarms_popup_event_handler(lv_event_t * e);
static void alarms_popup_delete(void);
static void alarms_popup_create_panel(void);
static void alarms_popup_create_buttons(uint32_t idx);
static void alarms_popup_create_text_area(char *text);
static void alarms_popup_create_labels(void);
static void alarms_popup_create_dropdowns(uint8_t hour, uint8_t minute);
static void alarms_popup_create_weekday_checkboxes(uint8_t flags);
static void alarms_popup_delete_weekday_checkboxes(void);
static void alarms_popup_show_hide_weekday_checkboxes(uint8_t show_hide);
static uint8_t alarm_popup_get_weekdays_checkboxes_flags(void);
static void alarms_popup_create_keyboard(void);
static void alarms_popup_show_hide_keyboard(uint8_t show_hide);
static void alarms_popup_set_new_alarm_values(uint8_t idx);

static void set_y(void *obj, int32_t val);
static void set_height(void *obj, int32_t val);
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a);
static void fill_buffer_for_dropdown_options(char *buff, uint8_t cnt);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
extern const char *Eng_DayName_Short[7];

static SemaphoreHandle_t alarms_popup_mutex_handle;

static UI_PopupObj_t alarms_popup;
static lv_obj_t *back_button, *ok_button, *alarm_text, *text_label, *time_label, *hours_dropdown, *minutes_dropdown, *keyboard;
static lv_obj_t **weekday_checkboxes_array;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize mutex for alarms popup operations */
void UI_AlarmsPopup_MutexInit(void){

	alarms_popup_mutex_handle = xSemaphoreCreateMutex();
	assert(alarms_popup_mutex_handle);
}

/* create default popup where user can edit the alarm */
void UI_AlarmsPopup_EditAlarm(uint8_t idx){

	BaseType_t res;
	AlarmData_t *alarm;

	alarm = Alarm_GetCurrentValues(idx);
	if(0 == alarm) return;

	res = xSemaphoreTake(alarms_popup_mutex_handle, pdMS_TO_TICKS(ALARMS_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	// delete the old popup if exists
	alarms_popup_delete();

	// create base popup panel
	alarms_popup_create_panel();

	// delete the label, we don't need it
	if(true == lv_obj_is_valid(alarms_popup.label)) {

		lv_obj_del(alarms_popup.label);
	}

	// set bigger size
	lv_obj_set_size(alarms_popup.panel, lv_pct(100), ALARMS_POPUP_HEIGH_FULL);

	// two buttons
	alarms_popup_create_buttons(idx);
	// text area
	alarms_popup_create_text_area(alarm->text);
	// two labels on the left side ("Text: Hour:")
	alarms_popup_create_labels();
	// two dropdowns with hours and minutes chocie
	alarms_popup_create_dropdowns(alarm->hour, alarm->minute);
	// seven checkboxes with weekday's selection
	alarms_popup_create_weekday_checkboxes(alarm->flags);
	// keyboard
	alarms_popup_create_keyboard();

	xSemaphoreGive(alarms_popup_mutex_handle);

	if(alarm->text){
		if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
	}
	free(alarm);
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* handler for ui alarms popup events */
static void alarms_popup_event_handler(lv_event_t * e){

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	BaseType_t res;
	uint32_t idx;

	res = xSemaphoreTake(alarms_popup_mutex_handle, pdMS_TO_TICKS(ALARMS_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	if((obj == back_button) && (LV_EVENT_RELEASED == code)){

		alarms_popup_delete();
	}

	// user request to change the alarm values
	else if((obj == ok_button) && (LV_EVENT_RELEASED == code)){

		idx = (uint32_t)lv_event_get_user_data(e);
		alarms_popup_set_new_alarm_values(idx);
		UI_ReportEvt(UI_EVT_ALARM_VALUES_CHANGED, (void *)idx);
		alarms_popup_delete();
	}

	// event occured at password text area
	else if(obj == alarm_text){

		if(LV_EVENT_FOCUSED == code){

			alarms_popup_show_hide_keyboard(show_keyboard);
		}
		else if(LV_EVENT_DEFOCUSED == code){

			alarms_popup_show_hide_keyboard(hide_keyboard);
		}
	}

	xSemaphoreGive(alarms_popup_mutex_handle);
}

/* delete all resources related to alarms popup */
static void alarms_popup_delete(void){

	if(true == lv_obj_is_valid(keyboard)) {

		lv_anim_del(keyboard, NULL);
		lv_obj_del(keyboard);
	}

	if(true == lv_obj_is_valid(alarms_popup.panel)) {

		lv_anim_del(alarms_popup.panel, NULL);
	}

	alarms_popup_delete_weekday_checkboxes();

	if(true == lv_obj_is_valid(alarms_popup.background)) {

		lv_obj_del(alarms_popup.background);
	}
}

/* create base of alarms popup object */
static void alarms_popup_create_panel(void){

	UI_PopupCreate(&alarms_popup);
}

/* create back and ok buttons */
static void alarms_popup_create_buttons(uint32_t idx){

	if(false == lv_obj_is_valid(alarms_popup.panel)) return;

	UI_ButtonCreate(&alarms_popup.panel, &back_button, ICON_LEFT_ARROW);
	lv_obj_set_align(back_button, LV_ALIGN_BOTTOM_LEFT);
	lv_obj_set_x(back_button, 10);
	lv_obj_add_event_cb(back_button, alarms_popup_event_handler, LV_EVENT_RELEASED, (void *)idx);

	UI_ButtonCreate(&alarms_popup.panel, &ok_button, ICON_RIGHT_ARROW);
	lv_obj_set_align(ok_button, LV_ALIGN_BOTTOM_RIGHT);
	lv_obj_set_x(ok_button, -10);
	lv_obj_add_event_cb(ok_button, alarms_popup_event_handler, LV_EVENT_RELEASED, (void *)idx);
}

/* create text area where user can enter text to be displayed */
static void alarms_popup_create_text_area(char *text){

	lv_style_value_t text_color;
	lv_res_t res;

	if(false == lv_obj_is_valid(alarms_popup.panel)) return;

	res = lv_style_get_prop(&UI_Text16Style, LV_STYLE_TEXT_COLOR, &text_color);
	if(LV_RES_OK != res) return;

	alarm_text = lv_textarea_create(alarms_popup.panel);
	lv_obj_add_style(alarm_text, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(alarm_text, UI_CurrentTheme.main_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);

	// set cursor to have the same color that UI_Text16Style font
	lv_obj_set_style_bg_color(alarm_text, text_color.color, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_set_style_bg_opa(alarm_text, LV_OPA_COVER, LV_PART_CURSOR | LV_STATE_FOCUSED);

    if(0 == text){

    	lv_textarea_set_text(alarm_text, "error");
    }
    else{

    	lv_textarea_set_text(alarm_text, text);
    }
    lv_textarea_set_password_mode(alarm_text, false);
    lv_textarea_set_one_line(alarm_text, true);
    lv_obj_set_width(alarm_text, lv_pct(80));
    lv_obj_set_align(alarm_text, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(alarm_text, 0);

	lv_obj_add_flag(alarm_text, (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE));
    lv_obj_add_event_cb(alarm_text, alarms_popup_event_handler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(alarm_text, alarms_popup_event_handler, LV_EVENT_DEFOCUSED, NULL);
}

/* two labels that describe text area and dropdowns function */
static void alarms_popup_create_labels(void){

	if(false == lv_obj_is_valid(alarms_popup.panel)) return;

	text_label = lv_label_create(alarms_popup.panel);
	lv_obj_add_style(text_label, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(text_label, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(text_label, 10);
    lv_label_set_text(text_label, "Text:");

	time_label = lv_label_create(alarms_popup.panel);
	lv_obj_add_style(time_label, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(time_label, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(time_label, 70);
    lv_label_set_text(time_label, "Time:");
}

/* two dropdowns where user can choose the time of an alarm */
static void alarms_popup_create_dropdowns(uint8_t hour, uint8_t minute){

	lv_obj_t *dropdown_list;
	char buff[256] = {0};

	if(false == lv_obj_is_valid(alarms_popup.panel)) return;

    hours_dropdown = lv_dropdown_create(alarms_popup.panel);
    lv_obj_add_style(hours_dropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(hours_dropdown, &UI_CheckboxStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(hours_dropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_align(hours_dropdown, LV_ALIGN_TOP_LEFT);
    lv_obj_set_x(hours_dropdown, 70);
    lv_obj_set_y(hours_dropdown, 60);
    lv_obj_set_width(hours_dropdown, 80);

    dropdown_list = lv_dropdown_get_list(hours_dropdown);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_CHECKED);

    fill_buffer_for_dropdown_options(buff, 24);
    lv_dropdown_set_options(hours_dropdown, buff);

    lv_dropdown_set_selected(hours_dropdown, hour);

    memset(buff, 0, 256);

    minutes_dropdown = lv_dropdown_create(alarms_popup.panel);
    lv_obj_add_style(minutes_dropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(minutes_dropdown, &UI_CheckboxStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(minutes_dropdown, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_align(minutes_dropdown, LV_ALIGN_TOP_LEFT);
    lv_obj_set_x(minutes_dropdown, 170);
    lv_obj_set_y(minutes_dropdown, 60);
    lv_obj_set_width(minutes_dropdown, 80);

    dropdown_list = lv_dropdown_get_list(minutes_dropdown);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_add_style(dropdown_list, &UI_DropdownStyle, LV_PART_SELECTED | LV_STATE_CHECKED);

    fill_buffer_for_dropdown_options(buff, 60);
    lv_dropdown_set_options(minutes_dropdown, buff);

    lv_dropdown_set_selected(minutes_dropdown, minute);
}

/* seven checkboxes which are used to select a weekdays when alarm will be repeated */
static void alarms_popup_create_weekday_checkboxes(uint8_t flags){

	uint8_t a;
	char buff[3];

	if(false == lv_obj_is_valid(alarms_popup.panel)) return;

	alarms_popup_delete_weekday_checkboxes();

	weekday_checkboxes_array = calloc(7, sizeof(lv_obj_t*));
	if(0 == weekday_checkboxes_array) return;

	for(a = 0; a < 7; a++){

		memset(buff, 0, 3);

		memcpy(buff, Eng_DayName_Short[a], 2);

		UI_CheckboxCreate(&alarms_popup.panel, &weekday_checkboxes_array[a], buff, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_align(weekday_checkboxes_array[a], LV_ALIGN_TOP_LEFT);
		lv_obj_set_x(weekday_checkboxes_array[a], (a * 40U));
		lv_obj_set_y(weekday_checkboxes_array[a], 120);

		if(flags & (1 << a)){

			lv_obj_add_state(weekday_checkboxes_array[a], LV_STATE_CHECKED);
		}
	}
}

/* delete all resources related to checkboxes */
static void alarms_popup_delete_weekday_checkboxes(void){

	if(0 == weekday_checkboxes_array) return;

	uint8_t a;

	for(a = 0; a < 7; a++){

		if(weekday_checkboxes_array[a]){

			if(true == lv_obj_is_valid(weekday_checkboxes_array[a])) {

				lv_obj_del(weekday_checkboxes_array[a]);
			}

			weekday_checkboxes_array[a] = 0;
		}
	}

	if(heap_caps_get_allocated_size(weekday_checkboxes_array)) free(weekday_checkboxes_array);
	weekday_checkboxes_array = 0;
}

/* show or hide the checkboxes */
static void alarms_popup_show_hide_weekday_checkboxes(uint8_t show_hide){

	if(0 == weekday_checkboxes_array) return;

	uint8_t a;

	for(a = 0; a < 7; a++){

		if(weekday_checkboxes_array[a]){

			if(true == lv_obj_is_valid(weekday_checkboxes_array[a])) {

				if(show_checkboxes == show_hide){

					lv_obj_clear_flag(weekday_checkboxes_array[a], LV_OBJ_FLAG_HIDDEN);
				}
				else if(hide_checkboxes == show_hide){

					lv_obj_add_flag(weekday_checkboxes_array[a], LV_OBJ_FLAG_HIDDEN);
				}
			}
		}
	}
}

/* chect states of all checkboxes and return as an uint8_t number */
static uint8_t alarm_popup_get_weekdays_checkboxes_flags(void){

	if(0 == weekday_checkboxes_array) return 0;

	uint8_t a, flags = 0;

	for(a = 0; a < 7; a++){

		if(LV_STATE_CHECKED == lv_obj_get_state(weekday_checkboxes_array[a])){

			flags |= (1U << a);
		}
	}

	return flags;
}

/* create keyboard outside the screen */
static void alarms_popup_create_keyboard(void){

	if(false == lv_obj_is_valid(alarms_popup.background)) return;

	/*Create a keyboard*/
	UI_KeyboardCreate(&alarms_popup.background, &keyboard);
}

/* show or hide keyboard on screen */
static void alarms_popup_show_hide_keyboard(uint8_t show_hide){

	if((false == lv_obj_is_valid(alarms_popup.panel)) || (false == lv_obj_is_valid(keyboard))) return;

	lv_anim_t popup_move, kb_move, popup_resize;
	uint32_t tmp = show_hide;

	// create panel animation
	lv_anim_init(&popup_move);
	lv_anim_set_exec_cb(&popup_move, set_y);
	lv_anim_set_var(&popup_move, alarms_popup.panel);
	lv_anim_set_time(&popup_move, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&popup_move, 0, -120);
	}
	else{

		lv_anim_set_values(&popup_move, -120, 0);
	}
	lv_anim_set_path_cb(&popup_move, lv_anim_path_ease_out);
	lv_anim_set_ready_cb(&popup_move, wifi_popup_show_keyboard_animation_ready);
	lv_anim_set_user_data(&popup_move, (void *)tmp);

	lv_anim_init(&popup_resize);
	lv_anim_set_exec_cb(&popup_resize, set_height);
	lv_anim_set_var(&popup_resize, alarms_popup.panel);
	lv_anim_set_time(&popup_resize, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&popup_resize, ALARMS_POPUP_HEIGH_FULL, ALARMS_POPUP_HEIGH_FOLDED);
	}
	else{

		lv_anim_set_values(&popup_resize, ALARMS_POPUP_HEIGH_FOLDED, ALARMS_POPUP_HEIGH_FULL);
	}
	lv_anim_set_path_cb(&popup_resize, lv_anim_path_ease_out);

	// create keyboard animation
	lv_anim_init(&kb_move);
	lv_anim_set_exec_cb(&kb_move, set_y);
	lv_anim_set_var(&kb_move, keyboard);
	lv_anim_set_time(&kb_move, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&kb_move, 240, 0);
		lv_keyboard_set_textarea(keyboard, alarm_text); /*Focus it on one of the text areas to start*/
	}
	else{

		lv_anim_set_values(&kb_move, 0, 240);
	}
	lv_anim_set_path_cb(&kb_move, lv_anim_path_ease_out);
	lv_anim_set_ready_cb(&kb_move, wifi_popup_show_keyboard_animation_ready);

	// start both
	if(show_keyboard == show_hide){

		alarms_popup_show_hide_weekday_checkboxes(hide_checkboxes);
	}
	lv_anim_start(&popup_move);
	lv_anim_start(&popup_resize);
	lv_anim_start(&kb_move);
}

/* report new values of an alarm */
static void alarms_popup_set_new_alarm_values(uint8_t idx){

	AlarmData_t *alarm;
	const char *text;
	int a;

	// get the copy of existing values structure
	alarm = Alarm_GetCurrentValues(idx);
	if(0 == alarm) return;

	// free memory where old text was stored
	if(alarm->text){
		if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
	}

	// set new values
	alarm->hour = lv_dropdown_get_selected(hours_dropdown);
	alarm->minute = lv_dropdown_get_selected(minutes_dropdown);

	// get the text entered by user
	text = lv_textarea_get_text(alarm_text);
	if(0 == alarm_text) goto error;

	// copy text to output data
	a = strlen(text);
	if(0 == a) goto error;
	alarm->text = malloc(a + 1);
	if(0 == alarm->text) goto error;
	memcpy(alarm->text, text, a + 1);

	// get values of checkoxes
	alarm->flags = alarm_popup_get_weekdays_checkboxes_flags();

	// report new values
	Alarm_SetValues(idx, alarm);
	return;

	error:
		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}
}

/* animate y position */
static void set_y(void *obj, int32_t val){

	lv_obj_set_y(obj, val);
}

/* animate y position */
static void set_height(void *obj, int32_t val){

	lv_obj_set_height(obj, val);
}

/* callback when animation of sliding keyboard is done */
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a){

	BaseType_t res;

	res = xSemaphoreTake(alarms_popup_mutex_handle, pdMS_TO_TICKS(ALARMS_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	if(a->var == alarms_popup.panel) {

		if(hide_keyboard == (uint32_t)a->user_data){

			alarms_popup_show_hide_weekday_checkboxes(show_checkboxes);
		}
		lv_anim_del(alarms_popup.panel, NULL);
	}
	else if(a->var == keyboard){

		lv_anim_del(keyboard, NULL);
	}

	xSemaphoreGive(alarms_popup_mutex_handle);
}

/* function that prepares the buffer to be set as a dropdown options list
 *
 * the format for LVGL dropdown is:
 * "option1\noption2\noption3\n"
 * the function prepares the list of cnt numbers, f.e. if cnt == 3 the lists is:
 * "0\n1\n2\n"
 *
 * */
static void fill_buffer_for_dropdown_options(char *buff, uint8_t cnt){

	uint8_t a;
	char tmpbuff[5];

	for(a = 0; a < cnt; a++){

		memset(tmpbuff, 0, 5);
		sprintf(tmpbuff, "%02d", a);
    	strcat(buff, tmpbuff);
    	strcat(buff, "\n");
	}
}
