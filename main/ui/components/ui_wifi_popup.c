#include "ui.h"
#include "ui_wifi_list.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define WIFI_POPUP_MUTEX_TIMEOUT_MS			100U
#define KEYBOARD_SHOW_HIDE_TIME_MS			300U
#define OK_NOK_LINE_ANIMATION_TIME_MS		2000U

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
enum { ok_line, nok_line };
enum { show_keyboard, hide_keyboard };

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void wifi_popup_event_handler(lv_event_t * e);
static void wifi_popup_delete(void);
static void wifi_popup_create_panel(void);
static void wifi_popup_create_line(uint8_t line_type);
static void wifi_popup_create_spinner(void);
static void wifi_popup_create_password_text_area(void);
static void wifi_popup_create_checkboxes(void);
static void wifi_popup_create_buttons(void *arg);
static void wifi_popup_create_keyboard(void);

static void wifi_popup_show_hide_keyboard(uint8_t show_hide);
static void wifi_popup_connect(void *arg);

static void set_line_opa(void *obj, int32_t val);
static void set_y(void *obj, int32_t val);
static void wifi_popup_line_animation_ready(struct _lv_anim_t *a);
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const lv_point_t ok_line_points[] = {{25, 25}, {50, 50}, {100, 0}};	// those make a "check" sign
static const lv_point_t nok_line_points[] = {{25, 0}, {75, 50}, {50, 25}, {75, 0}, {25, 50}};	// those make an "X" sign

static SemaphoreHandle_t wifi_popup_mutex_handle;

static lv_obj_t *spinner, *wifi_popup_line, *password_text,
				*save_checkbox, *hide_checkbox, *back_button, *ok_button, *keyboard;

static UI_PopupObj_t wifi_popup;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void UI_WifiPopup_MutexInit(void){

	wifi_popup_mutex_handle = xSemaphoreCreateMutex();
	assert(wifi_popup_mutex_handle);
}

void UI_WifiPopup_Delete(void){

	BaseType_t res;

	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup when wifi connection is in progress */
void UI_WifiPopup_Connecting(char *ssid){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	if(0 != ssid){

		// prepare text
	    lv_label_set_text_fmt(wifi_popup.label, "Connecting to:\n%s", ssid);
	}
	else{

		lv_label_set_text(wifi_popup.label, "Connecting...");
	}

    wifi_popup_create_spinner();

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup when wifi has been connected */
void UI_WifiPopup_Connected(char *ssid){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	// prepare text
	if(0 == ssid){

		lv_label_set_text(wifi_popup.label, "Connected!");
	}
	else{

		lv_label_set_text_fmt(wifi_popup.label, "Connected to:\n%s", ssid);
	}

	wifi_popup_create_line(ok_line);

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup when wifi has not been connected */
void UI_WifiPopup_NotConnected(void){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	// prepare text
	lv_label_set_text(wifi_popup.label, "Connecting error!");

	wifi_popup_create_line(nok_line);

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup where user can enter the password */
void UI_WifiPopup_GetPass(WifiCreds_t *creds){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	// set bigger size
	lv_obj_set_size(wifi_popup.panel, LV_PCT(100), LV_PCT(50));

	lv_label_set_text(wifi_popup.label, "Password:");

    /*Create the rest of objects */
	wifi_popup_create_password_text_area();

	wifi_popup_create_checkboxes();

	wifi_popup_create_buttons(creds);

	wifi_popup_create_keyboard();

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup when password for given SSID name has been found and deleted */
void UI_WifiPopup_PassDeleted(char *ssid){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	// prepare text
	if(0 == ssid){

		lv_label_set_text(wifi_popup.label, "Password deleted!");
	}
	else{

		lv_label_set_text_fmt(wifi_popup.label, "Deleted password for \n%s", ssid);
	}

	wifi_popup_create_line(ok_line);

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* popup when password for given SSID name has not been found */
void UI_WifiPopup_PassNotDeleted(char *ssid){

	BaseType_t res;

	// mutex for wifi popup
	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	wifi_popup_delete();

	wifi_popup_create_panel();

	// prepare text
	if(0 == ssid){

		lv_label_set_text(wifi_popup.label, "Password not stored!");
	}
	else{

		lv_label_set_text_fmt(wifi_popup.label, "No password for \n%s!", ssid);
	}

	wifi_popup_create_line(nok_line);

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/
static void wifi_popup_event_handler(lv_event_t * e){

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	BaseType_t res;

	res = xSemaphoreTake(wifi_popup_mutex_handle, pdMS_TO_TICKS(WIFI_POPUP_MUTEX_TIMEOUT_MS));
	if(pdFALSE == res) return;

	// event occured at password text area
	if(obj == password_text){

		if(LV_EVENT_FOCUSED == code){

			wifi_popup_show_hide_keyboard(show_keyboard);	//1
		}
		else if(LV_EVENT_DEFOCUSED == code){

			wifi_popup_show_hide_keyboard(hide_keyboard);
		}
	}
	// hide checkbox has been toggled
	else if((obj == hide_checkbox) && (LV_EVENT_VALUE_CHANGED == code)){

		if(LV_STATE_CHECKED & lv_obj_get_state(obj)){

			lv_textarea_set_password_mode(password_text, true);		//2
		}
		else{

			lv_textarea_set_password_mode(password_text, false);
		}
	}
	// back button at wifi popup panel has been clicked
	else if((obj == back_button) && (LV_EVENT_RELEASED == code)){

		// free resources
		WifiCreds_t *creds = lv_event_get_user_data(e);
		if(creds){
			if(creds->ssid){
				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
			}
			if(creds->pass){
				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
			}
			if(heap_caps_get_allocated_size(creds)) free(creds);
		}

		// delete popup
		wifi_popup_delete();
	}

	// ok button at wifi popup panel has been clicked
	else if((obj == ok_button) && (LV_EVENT_RELEASED == code)){

		wifi_popup_connect(lv_event_get_user_data(e));
		wifi_popup_delete();
	}

	xSemaphoreGive(wifi_popup_mutex_handle);
}

/* remove all resources related to wifi popup */
static void wifi_popup_delete(void){

	if(true == lv_obj_is_valid(keyboard)) {

		lv_anim_del(keyboard, NULL);
		lv_obj_del(keyboard);
	}

	if(true == lv_obj_is_valid(wifi_popup_line)) {

		lv_anim_del(wifi_popup_line, NULL);
	}

	if(true == lv_obj_is_valid(wifi_popup.panel)) {

		lv_anim_del(wifi_popup.panel, NULL);
	}

	if(true == lv_obj_is_valid(wifi_popup.background)) {

		lv_obj_del(wifi_popup.background);
	}
}

/* create base of wifi popup object */
static void wifi_popup_create_panel(void){

	UI_PopupCreate(&wifi_popup);
}

/* create "sign" based on line object */
static void wifi_popup_create_line(uint8_t line_type){

	lv_anim_t line_opa_anim;

	if(false == lv_obj_is_valid(wifi_popup.panel)) return;

	// create an object
	wifi_popup_line = lv_line_create(wifi_popup.panel);
	lv_obj_set_align(wifi_popup_line, LV_ALIGN_BOTTOM_MID);
	lv_obj_set_x(wifi_popup_line, -5);
	lv_obj_set_y(wifi_popup_line, -10);
	lv_obj_set_style_line_opa(wifi_popup_line, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_line_color(wifi_popup_line, UI_CurrentTheme.contrast_color, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_line_width(wifi_popup_line, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_line_rounded(wifi_popup_line, true, LV_PART_MAIN | LV_STATE_DEFAULT);

	if(ok_line == line_type){

		lv_line_set_points(wifi_popup_line, ok_line_points, 3);
	}
	else{

		lv_line_set_points(wifi_popup_line, nok_line_points, 5);
	}

	// create and start animation
	lv_anim_init(&line_opa_anim);
	lv_anim_set_exec_cb(&line_opa_anim, set_line_opa);
	lv_anim_set_var(&line_opa_anim, wifi_popup_line);
	lv_anim_set_time(&line_opa_anim, OK_NOK_LINE_ANIMATION_TIME_MS);
	lv_anim_set_values(&line_opa_anim, 0, 255);
	lv_anim_set_path_cb(&line_opa_anim, lv_anim_path_ease_out);
	lv_anim_set_ready_cb(&line_opa_anim, wifi_popup_line_animation_ready);

	lv_anim_start(&line_opa_anim);
}

/* create spinner */
static void wifi_popup_create_spinner(void){

	if(false == lv_obj_is_valid(wifi_popup.panel)) return;

	spinner = lv_spinner_create(wifi_popup.panel, 1000, 60);
	lv_obj_set_size(spinner, 50, 50);
	lv_obj_set_align(spinner, LV_ALIGN_BOTTOM_MID);
	lv_obj_set_y(spinner, -10);
	lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);      /// Flags
	lv_obj_set_style_arc_opa(spinner, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(spinner, UI_CurrentTheme.main_color_ext, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

/* create text area where user can enter password */
static void wifi_popup_create_password_text_area(void){

	lv_style_value_t text_color;
	lv_res_t res;

	if(false == lv_obj_is_valid(wifi_popup.panel)) return;

	res = lv_style_get_prop(&UI_Text16Style, LV_STYLE_TEXT_COLOR, &text_color);
	if(LV_RES_OK != res) return;

	password_text = lv_textarea_create(wifi_popup.panel);
	lv_obj_add_style(password_text, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(password_text, UI_CurrentTheme.main_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);

	// set cursor to have the same color that UI_Text16Style font
	lv_obj_set_style_bg_color(password_text, text_color.color, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_set_style_bg_opa(password_text, LV_OPA_COVER, LV_PART_CURSOR | LV_STATE_FOCUSED);

    lv_textarea_set_text(password_text, "");
    lv_textarea_set_password_mode(password_text, true);
    lv_textarea_set_one_line(password_text, true);
    lv_obj_set_width(password_text, lv_pct(80));
    lv_obj_set_align(password_text, LV_ALIGN_TOP_MID);
    lv_obj_set_y(password_text, 30);

	lv_obj_add_flag(password_text, (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE));
    lv_obj_add_event_cb(password_text, wifi_popup_event_handler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(password_text, wifi_popup_event_handler, LV_EVENT_DEFOCUSED, NULL);
}

/* create two checkboxes to hide and save password */
static void wifi_popup_create_checkboxes(void){

	if(false == lv_obj_is_valid(wifi_popup.panel)) return;

	UI_CheckboxCreate(&wifi_popup.panel, &save_checkbox, "Save");
	lv_obj_set_align(save_checkbox, LV_ALIGN_LEFT_MID);
	lv_obj_set_x(save_checkbox, lv_pct(15));
    lv_obj_add_state(save_checkbox, LV_STATE_CHECKED);

    UI_CheckboxCreate(&wifi_popup.panel, &hide_checkbox, "Hide");
	lv_obj_set_align(hide_checkbox, LV_ALIGN_LEFT_MID);
	lv_obj_set_x(hide_checkbox, lv_pct(60));
    lv_obj_add_state(hide_checkbox, LV_STATE_CHECKED);
    lv_obj_add_event_cb(hide_checkbox, wifi_popup_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
}

/* create back and connect buttons */
static void wifi_popup_create_buttons(void *arg){

	if(false == lv_obj_is_valid(wifi_popup.panel)) return;

	UI_ButtonCreate(&wifi_popup.panel, &back_button, ICON_LEFT_ARROW);
	lv_obj_set_align(back_button, LV_ALIGN_BOTTOM_LEFT);
	lv_obj_set_x(back_button, 10);
	lv_obj_add_event_cb(back_button, wifi_popup_event_handler, LV_EVENT_RELEASED, arg);

	UI_ButtonCreate(&wifi_popup.panel, &ok_button, ICON_RIGHT_ARROW);
	lv_obj_set_align(ok_button, LV_ALIGN_BOTTOM_RIGHT);
	lv_obj_set_x(ok_button, -10);
	lv_obj_add_event_cb(ok_button, wifi_popup_event_handler, LV_EVENT_RELEASED, arg);
}

/* create keyboard outside the screen */
static void wifi_popup_create_keyboard(void){

	if(false == lv_obj_is_valid(wifi_popup.background)) return;

	/*Create a keyboard*/
	keyboard = lv_keyboard_create(wifi_popup.background);
	lv_obj_set_align(keyboard, LV_ALIGN_BOTTOM_MID);
	lv_obj_set_y(keyboard, 240);
	lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(50));

	// keyboard background
	lv_obj_set_style_bg_color(keyboard, UI_CurrentTheme.background_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(keyboard, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	// background for normal keys
	lv_obj_set_style_bg_color(keyboard, UI_CurrentTheme.background_color_ext, LV_PART_ITEMS | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(keyboard, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);

	// background for checked keys
	lv_obj_set_style_bg_color(keyboard, UI_CurrentTheme.background_color_base, LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(keyboard, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);

	// border for all keys
	lv_obj_set_style_border_width(keyboard, 1, LV_PART_ITEMS);
	lv_obj_set_style_border_color(keyboard, UI_CurrentTheme.main_color_base, LV_PART_ITEMS);
	lv_obj_set_style_border_opa(keyboard, LV_OPA_60, LV_PART_ITEMS);

	// text color for all keys
	lv_obj_set_style_text_color(keyboard, UI_CurrentTheme.main_color_ext, LV_PART_ITEMS);
	lv_obj_set_style_text_color(keyboard, UI_CurrentTheme.main_color_ext, LV_PART_ITEMS | LV_STATE_CHECKED);
}

/* show or hide keyboard on screen */
static void wifi_popup_show_hide_keyboard(uint8_t show_hide){

	lv_anim_t popup_move, kb_move;

	// create panel animation
	lv_anim_init(&popup_move);
	lv_anim_set_exec_cb(&popup_move, set_y);
	lv_anim_set_var(&popup_move, wifi_popup.panel);
	lv_anim_set_time(&popup_move, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&popup_move, 0, -120);
	}
	else{

		lv_anim_set_values(&popup_move, -120, 0);
	}
	lv_anim_set_path_cb(&popup_move, lv_anim_path_ease_out);
	lv_anim_set_ready_cb(&popup_move, wifi_popup_show_keyboard_animation_ready);

	// create keyboard animation
	lv_anim_init(&kb_move);
	lv_anim_set_exec_cb(&kb_move, set_y);
	lv_anim_set_var(&kb_move, keyboard);
	lv_anim_set_time(&kb_move, KEYBOARD_SHOW_HIDE_TIME_MS);
	if(show_keyboard == show_hide){

		lv_anim_set_values(&kb_move, 240, 0);
		lv_keyboard_set_textarea(keyboard, password_text); /*Focus it on one of the text areas to start*/
	}
	else{

		lv_anim_set_values(&kb_move, 0, 240);
	}
	lv_anim_set_path_cb(&kb_move, lv_anim_path_ease_out);
	lv_anim_set_ready_cb(&kb_move, wifi_popup_show_keyboard_animation_ready);

	// start both
	lv_anim_start(&popup_move);
	lv_anim_start(&kb_move);
}

/* function called when user clicked "next" button after entering the wifi password */
static void wifi_popup_connect(void *arg){

	if(0 == arg) return;

	int a;
	const char *pass = 0;
	WifiCreds_t *creds = (WifiCreds_t *)arg;

	// get the password entered by user
	pass = lv_textarea_get_text(password_text);
	if(0 == pass) goto error;

	// copy password to output data
	a = strnlen(pass, 64);
	if((0 == a) || (64 == a)) goto error;
	creds->pass = malloc(a + 1);
	if(0 == creds->pass) goto error;
	memcpy(creds->pass, pass, a + 1);

	// set if user requested to save the password
	if(LV_STATE_CHECKED == lv_obj_get_state(save_checkbox)){

		creds->save = true;
	}
	else{

		creds->save = false;
	}

	Wifi_Connect(creds);
	return;

	error:
		if(creds){
			if(creds->ssid){
				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
			}
			if(creds->pass){
				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
			}
			if(heap_caps_get_allocated_size(creds)) free(creds);
		}
}
/**************************************************************
 * Animation helpers
 ***************************************************************/

/* animate line opacity */
static void set_line_opa(void *obj, int32_t val){

	lv_obj_set_style_line_opa(obj, val, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* animate y position */
static void set_y(void *obj, int32_t val){

	lv_obj_set_y(obj, val);
}

/* callback when animation of wifi popup "sign" is done */
static void wifi_popup_line_animation_ready(struct _lv_anim_t *a){

	UI_ReportEvt(UI_EVT_WIFI_POPUP_DELETE_REQUEST, 0);
}

/* callback when animation of sliding keyboard is done */
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a){

	if(a->var == wifi_popup.panel)lv_anim_del(wifi_popup.panel, NULL);
	else if(a->var == keyboard)lv_anim_del(keyboard, NULL);
}
