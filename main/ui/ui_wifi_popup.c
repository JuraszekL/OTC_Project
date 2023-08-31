#include "lvgl.h"
#include "ui.h"
#include "ui_task.h"
#include "wifi.h"
#include "ui_styles.h"

/**************************************************************
 *
 *	Defines
 *
 ***************************************************************/
#define KEYBOARD_SHOW_HIDE_TIME_MS		300U

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
static void wifi_popup_create_panel(void);
static void wifi_popup_create_line(uint8_t line_type);
static void wifi_popup_create_spinner(void);
static void wifi_popup_create_password_text_area(void);
static void wifi_popup_create_checkboxes(void);
static void wifi_popup_create_buttons(void);
static void wifi_popup_create_keyboard(void);

static void wifi_popup_show_hide_keyboard(uint8_t show_hide);

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

static lv_obj_t *top_background, *panel, *top_text, *spinner, *wifi_popup_line, *password_text,
				*save_checkbox, *hide_checkbox, *back_button, *ok_button, *keyboard;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void UI_WifiPopup_Connecting(char *ssid){

	char buff[64] = {0};

	// check if popup base is valid
	if(false == lv_obj_is_valid(top_background)){

		wifi_popup_create_panel();
	}

	// delete popup "sign" if exists
	if(true == lv_obj_is_valid(wifi_popup_line)){

		lv_obj_del(wifi_popup_line);
	}

	// prepare text
	sprintf(buff, "Connecting to:\n%s", ssid);
    lv_label_set_text(top_text, buff);

    wifi_popup_create_spinner();

}

void UI_WifiPopup_Connected(char *ssid){

	char buff[64] = {0};

	// check if popup base is valid
	if(false == lv_obj_is_valid(top_background)){

		wifi_popup_create_panel();
	}

	// delete popup spinner if exists
	if(true == lv_obj_is_valid(spinner)){

		lv_obj_del(spinner);
	}

	// prepare text
	if(0 == ssid){

		lv_label_set_text(top_text, "Connected!");
	}
	else{

		sprintf(buff, "Connected to:\n%s", ssid);
	    lv_label_set_text(top_text, buff);
	}

	wifi_popup_create_line(ok_line);
}

void UI_WifiPopup_NotConnected(void){

	// check if popup base is valid
	if(false == lv_obj_is_valid(top_background)){

		wifi_popup_create_panel();
	}

	// delete popup spinner if exists
	if(true == lv_obj_is_valid(spinner)){

		lv_obj_del(spinner);
		spinner = 0;
	}

	// prepare text
	lv_label_set_text(top_text, "Connecting error!");

	wifi_popup_create_line(nok_line);
}

/* create popup where user can enter the password */
void UI_WifiPopup_GetPass(WifiCreds_t *creds){

	// check if popup base is valid
	if(false == lv_obj_is_valid(top_background)){

		wifi_popup_create_panel();
	}

	// set bigger size
	lv_obj_set_size(panel, LV_PCT(100), LV_PCT(50));

	lv_label_set_text(top_text, "Password:");

	// delete popup spinner if exists
	if(true == lv_obj_is_valid(spinner)){

		lv_obj_del(spinner);
		spinner = 0;
	}

    /*Create the rest of objects */
	wifi_popup_create_password_text_area();

	wifi_popup_create_checkboxes();

	wifi_popup_create_buttons();

	wifi_popup_create_keyboard();
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/
static void wifi_popup_event_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    // event occured at password text area
    if(obj == password_text){

    	if(LV_EVENT_FOCUSED == code){

    		wifi_popup_show_hide_keyboard(show_keyboard);
    	}
    	else if(LV_EVENT_DEFOCUSED == code){

    		wifi_popup_show_hide_keyboard(hide_keyboard);
    	}
    }
    // hide checkbox has been toggled
    else if((obj == hide_checkbox) && (LV_EVENT_VALUE_CHANGED == code)){

    	if(LV_STATE_CHECKED & lv_obj_get_state(obj)){

    		lv_textarea_set_password_mode(password_text, true);
    	}
    	else{

    		lv_textarea_set_password_mode(password_text, false);
    	}
    }
    // back button at wifi popup panel has been clicked
    else if((obj == back_button) && (LV_EVENT_PRESSED == code)){

    	lv_obj_del_async(top_background);
    }
}

/* create base of wifi popup object */
static void wifi_popup_create_panel(void){

	top_background = lv_obj_create(lv_layer_top());
	lv_obj_add_style(top_background, &UI_PopupPanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_size(top_background, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_bg_opa(top_background, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(top_background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_clear_flag(top_background, LV_OBJ_FLAG_SCROLLABLE);

	panel = lv_obj_create(top_background);
	lv_obj_add_style(panel, &UI_PopupPanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_size(panel, 200, 150);
	lv_obj_set_align(panel, LV_ALIGN_CENTER);

	top_text = lv_label_create(panel);
	lv_obj_add_style(top_text, &UI_PopupPanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(top_text, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(top_text, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(top_text, LV_ALIGN_TOP_MID);
    lv_obj_set_style_text_align(top_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* create "sign" based on line object */
static void wifi_popup_create_line(uint8_t line_type){

	lv_anim_t line_opa_anim;

	// create an object
	wifi_popup_line = lv_line_create(panel);
	lv_obj_set_align(wifi_popup_line, LV_ALIGN_BOTTOM_MID);
	lv_obj_add_style(wifi_popup_line, &UI_PopupPanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_line_opa(wifi_popup_line, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

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
	lv_anim_set_time(&line_opa_anim, 600);
	lv_anim_set_values(&line_opa_anim, 0, 255);
	lv_anim_set_path_cb(&line_opa_anim, lv_anim_path_ease_in);
	lv_anim_set_ready_cb(&line_opa_anim, wifi_popup_line_animation_ready);

	lv_anim_start(&line_opa_anim);
}

/* create spinner */
static void wifi_popup_create_spinner(void){

	spinner = lv_spinner_create(panel, 1000, 60);
	lv_obj_set_size(spinner, 50, 50);
	lv_obj_set_align(spinner, LV_ALIGN_BOTTOM_MID);
	lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);      /// Flags
	lv_obj_set_style_arc_color(spinner, lv_color_hex(0x262223), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(spinner, lv_color_hex(0xF26B1D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

/* create text area where user can enter password */
static void wifi_popup_create_password_text_area(void){

	password_text = lv_textarea_create(panel);
	lv_obj_add_style(password_text, &UI_PopupPanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_style(password_text, &UI_PopupPanelStyle, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_textarea_set_text(password_text, "");
    lv_textarea_set_password_mode(password_text, true);
    lv_textarea_set_one_line(password_text, true);
    lv_obj_set_width(password_text, lv_pct(80));
    lv_obj_set_align(password_text, LV_ALIGN_TOP_MID);
    lv_obj_set_y(password_text, 30);
//    lv_obj_set_style_bg_color(password_text, lv_color_hex(0xF2F2F2), 0);
    lv_obj_add_event_cb(password_text, wifi_popup_event_handler, LV_EVENT_ALL, NULL);
}

/* create two checkboxes to hide and save password */
static void wifi_popup_create_checkboxes(void){

	save_checkbox = lv_checkbox_create(panel);
	lv_obj_add_style(save_checkbox, &UI_PopupPanelStyle, LV_PART_MAIN | LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_add_style(save_checkbox, &UI_PopupPanelStyle, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_align(save_checkbox, LV_ALIGN_LEFT_MID);
	lv_obj_set_x(save_checkbox, lv_pct(15));
    lv_checkbox_set_text(save_checkbox, "Save");
    lv_obj_add_state(save_checkbox, LV_STATE_CHECKED);

	hide_checkbox = lv_checkbox_create(panel);
	lv_obj_add_style(hide_checkbox, &UI_PopupPanelStyle, LV_PART_MAIN | LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_add_style(hide_checkbox, &UI_PopupPanelStyle, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_align(hide_checkbox, LV_ALIGN_LEFT_MID);
	lv_obj_set_x(hide_checkbox, lv_pct(60));
    lv_checkbox_set_text(hide_checkbox, "Hide");
    lv_obj_add_state(hide_checkbox, LV_STATE_CHECKED);
    lv_obj_add_event_cb(hide_checkbox, wifi_popup_event_handler, LV_EVENT_ALL, NULL);
}

/* create back and connect buttons */
static void wifi_popup_create_buttons(void){

	lv_obj_t *but_label;

	back_button = lv_btn_create(panel);
	lv_obj_add_style(back_button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(back_button, LV_ALIGN_BOTTOM_LEFT);
	lv_obj_add_event_cb(back_button, wifi_popup_event_handler, LV_EVENT_ALL, NULL);

	but_label = lv_label_create(back_button);
	lv_obj_add_style(but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_label_set_text_fmt(but_label, "%c", ICON_LEFT_ARROW);

	ok_button = lv_btn_create(panel);
	lv_obj_add_style(ok_button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(ok_button, LV_ALIGN_BOTTOM_RIGHT);
	lv_obj_add_event_cb(ok_button, wifi_popup_event_handler, LV_EVENT_ALL, NULL);

	but_label = lv_label_create(ok_button);
	lv_obj_add_style(but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_label_set_text_fmt(but_label, "%c", ICON_RIGHT_ARROW);
}

static void wifi_popup_create_keyboard(void){

	/*Create a keyboard*/
	keyboard = lv_keyboard_create(top_background);
	lv_obj_set_align(keyboard, LV_ALIGN_BOTTOM_MID);
	lv_obj_set_y(keyboard, 240);
	lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(50));

	// bacground dark
	lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x262223), LV_PART_MAIN | LV_STATE_DEFAULT);

	// background dark for normal keys
	lv_obj_set_style_bg_opa(keyboard, LV_OPA_TRANSP, LV_PART_ITEMS);

	// background for checked keys
	lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xF26B1D), LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(keyboard, LV_OPA_10, LV_PART_ITEMS | LV_STATE_CHECKED);

	// border for all keys
	lv_obj_set_style_border_width(keyboard, 1, LV_PART_ITEMS);
	lv_obj_set_style_border_color(keyboard, lv_color_hex(0xF26B1D), LV_PART_ITEMS);
	lv_obj_set_style_border_opa(keyboard, LV_OPA_60, LV_PART_ITEMS);

	// text color for all keys
	lv_obj_set_style_text_color(keyboard, lv_color_hex(0xF2921D), LV_PART_ITEMS);
	lv_obj_set_style_text_color(keyboard, lv_color_hex(0xF2921D), LV_PART_ITEMS | LV_STATE_CHECKED);
}

static void wifi_popup_show_hide_keyboard(uint8_t show_hide){

	lv_anim_t popup_move, kb_move;

	// create panel animation
	lv_anim_init(&popup_move);
	lv_anim_set_exec_cb(&popup_move, set_y);
	lv_anim_set_var(&popup_move, panel);
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

	lv_anim_del(wifi_popup_line, NULL);
	lv_obj_del_delayed(top_background, 500);
}

/* callback when animation of sliding keyboard is done */
static void wifi_popup_show_keyboard_animation_ready(struct _lv_anim_t *a){

	if(a->var == panel)lv_anim_del(panel, NULL);
	else if(a->var == keyboard)lv_anim_del(keyboard, NULL);
}
