#include "ui.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define ALARMS_POPUP_MUTEX_TIMEOUT_MS			100U

enum { show_keyboard, hide_keyboard };

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void alarms_popup_event_handler(lv_event_t * e);
static void alarms_popup_delete(void);
static void alarms_popup_create_panel(void);
static void alarms_popup_create_buttons(uint32_t idx);
static void alarms_popup_create_text_area(void);
static void alarms_popup_create_keyboard(void);
static void alarms_popup_show_hide_keyboard(uint8_t show_hide);

static void set_y(void *obj, int32_t val);
static void set_height(void *obj, int32_t val);
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static SemaphoreHandle_t alarms_popup_mutex_handle;

static UI_PopupObj_t alarms_popup;
static lv_obj_t *back_button, *ok_button, *alarm_text, *keyboard;


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void UI_AlarmsPopup_MutexInit(void){

	alarms_popup_mutex_handle = xSemaphoreCreateMutex();
	assert(alarms_popup_mutex_handle);
}

void UI_AlarmsPopup_EditAlarm(uint8_t idx){

	BaseType_t res;

	res = xSemaphoreTake(alarms_popup_mutex_handle, pdMS_TO_TICKS(ALARMS_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	alarms_popup_delete();

	alarms_popup_create_panel();

	if(true == lv_obj_is_valid(alarms_popup.label)) {

		lv_obj_del(alarms_popup.label);
	}

	// set bigger size
	lv_obj_set_size(alarms_popup.panel, 320, 320);

	alarms_popup_create_buttons(idx);

	alarms_popup_create_text_area();

	alarms_popup_create_keyboard();

	xSemaphoreGive(alarms_popup_mutex_handle);
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

	res = xSemaphoreTake(alarms_popup_mutex_handle, pdMS_TO_TICKS(ALARMS_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	if((obj == back_button) && (LV_EVENT_RELEASED == code)){

		alarms_popup_delete();
	}

	else if((obj == ok_button) && (LV_EVENT_RELEASED == code)){

		//
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
static void alarms_popup_create_text_area(void){

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

    lv_textarea_set_text(alarm_text, "");
    lv_textarea_set_password_mode(alarm_text, false);
    lv_textarea_set_one_line(alarm_text, true);
    lv_obj_set_width(alarm_text, lv_pct(80));
    lv_obj_set_align(alarm_text, LV_ALIGN_TOP_MID);
    lv_obj_set_y(alarm_text, 10);

	lv_obj_add_flag(alarm_text, (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE));
    lv_obj_add_event_cb(alarm_text, alarms_popup_event_handler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(alarm_text, alarms_popup_event_handler, LV_EVENT_DEFOCUSED, NULL);
}

/* create keyboard outside the screen */
static void alarms_popup_create_keyboard(void){

	if(false == lv_obj_is_valid(alarms_popup.background)) return;

	/*Create a keyboard*/
	UI_KeyboardCreate(&alarms_popup.background, &keyboard);
}

/* show or hide keyboard on screen */
static void alarms_popup_show_hide_keyboard(uint8_t show_hide){

	lv_anim_t popup_move, kb_move, popup_resize;

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

	lv_anim_init(&popup_resize);
	lv_anim_set_exec_cb(&popup_resize, set_height);
	lv_anim_set_var(&popup_resize, alarms_popup.panel);
	lv_anim_set_time(&popup_resize, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&popup_resize, 320, 240);
	}
	else{

		lv_anim_set_values(&popup_resize, 240, 320);
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
	lv_anim_start(&popup_move);
	lv_anim_start(&popup_resize);
	lv_anim_start(&kb_move);
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

	if(a->var == alarms_popup.panel)lv_anim_del(alarms_popup.panel, NULL);
	else if(a->var == keyboard)lv_anim_del(keyboard, NULL);
}
