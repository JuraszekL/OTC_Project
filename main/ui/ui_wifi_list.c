#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_task.h"
#include "spiffs_task.h"
#include "wifi.h"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
struct list_objects {

	lv_obj_t *obj;
	struct list_objects *next;
};

enum { ok_line, nok_line };

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void wifi_list_event_handler(lv_event_t * e);
static void wifi_popup_create_panel(void);
static void wifi_popup_create_line(uint8_t line_type);
static void wifi_popup_create_spinner(void);

static void set_line_opa(void *obj, int32_t val);
static void wifi_popup_line_animation_ready(struct _lv_anim_t *a);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const lv_point_t ok_line_points[] = {{25, 25}, {50, 50}, {100, 0}};	// those make a "check" sign
static const lv_point_t nok_line_points[] = {{25, 0}, {75, 50}, {50, 25}, {75, 0}, {25, 50}};	// those make an "X" sign

static lv_style_t style_list, style_popup;
static lv_obj_t *wifi_list, *top_background, *panel, *top_text, *spinner, *wifi_popup_line;
static struct list_objects *wifi_list_objects;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize wifi list component */
void UI_WifiListInit(void){

	if(true == lv_obj_is_valid(wifi_list)) return;	//skip if already initialized

	// init list style
	lv_style_init(&style_list);
	lv_style_set_bg_opa(&style_list, LV_OPA_TRANSP);
	lv_style_set_border_opa(&style_list, LV_OPA_TRANSP);
	lv_style_set_text_font(&style_list, &lv_font_montserrat_16);
	lv_style_set_text_color(&style_list, lv_color_hex(0xF2921D));

	lv_style_init(&style_popup);
	lv_style_set_bg_color(&style_popup, lv_color_hex(0x262223));
	lv_style_set_bg_opa(&style_popup, LV_OPA_COVER);
	lv_style_set_border_color(&style_popup, lv_color_hex(0xF26B1D));
	lv_style_set_border_opa(&style_popup, LV_OPA_COVER);
	lv_style_set_text_font(&style_popup, &lv_font_montserrat_16);
	lv_style_set_text_color(&style_popup, lv_color_hex(0xF2921D));
	lv_style_set_line_color(&style_popup, lv_color_hex(0xF2921D));
	lv_style_set_line_width(&style_popup, 8);
	lv_style_set_line_rounded(&style_popup, true);

	// init list
	wifi_list = lv_list_create(ui_WifiScreen);
    lv_obj_set_size(wifi_list, 320, 250);
    lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 145);

    lv_obj_add_style(wifi_list, &style_list, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* clear whole list */
void UI_WifiListClear(void){

	if(0 == wifi_list_objects) return;	// skip if list is emmpty

	struct list_objects *current, *next;

	current = wifi_list_objects;

	// delete all objects and free alocated memory
	do{
		next = current->next;
		if(true == lv_obj_is_valid(current->obj)){

			lv_obj_del(current->obj);
			current->obj = 0;
		}
		free(current);

		current = next;

	} while(0 != next);

	wifi_list_objects = 0;
}

void UI_WifiListAdd(bool is_protected, char *name, int rssi){

	if(false == lv_obj_is_valid(wifi_list)) return;	//skip if list is not initialized

	struct list_objects *new_obj;
	lv_obj_t *button_icon_right;
	char rssi_icon;

	// if list is empty
	if(0 == wifi_list_objects){

		wifi_list_objects = calloc(1, sizeof(struct list_objects));
		if(0 == wifi_list_objects) return;
		new_obj = wifi_list_objects;
	}
	else{	// if isn't

		new_obj = wifi_list_objects;
		while(0 != new_obj->next){

			new_obj = new_obj->next;

		}

		new_obj->next = calloc(1, sizeof(struct list_objects));
		if(0 == wifi_list_objects) return;
		new_obj = new_obj->next;
	}

	// add button with wifi name
	new_obj->obj = lv_list_add_btn(wifi_list, 0, name);
	lv_obj_add_event_cb(new_obj->obj, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(new_obj->obj, &style_list, LV_PART_MAIN | LV_STATE_DEFAULT);

	// add right aligned label to the button
	button_icon_right = lv_label_create(new_obj->obj);
    lv_obj_set_width(button_icon_right, LV_SIZE_CONTENT);
    lv_obj_set_height(button_icon_right, LV_SIZE_CONTENT);
    lv_obj_set_align(button_icon_right, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_align(button_icon_right, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button_icon_right, &ui_font_UIIconsNew16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // set signal icon according to rssi
    if(rssi >= (int)-50){

    	rssi_icon = ICON_SIGNAL_FULL;
    }
    else if(((int)-60 <= rssi) && ( rssi < (int)-50)){

    	rssi_icon = ICON_SIGNAL_GOOD;
    }
    else if(((int)-70 <= rssi) && ( rssi < (int)-60)){

    	rssi_icon = ICON_SIGNAL_MID;
    }
    else rssi_icon = ICON_SIGNAL_LOW;

    // set additional padlock icon if wifi is protected
    if(true == is_protected){

    	lv_label_set_text_fmt(button_icon_right, "%c %c",ICON_PADLOCK, rssi_icon);
    }
    else{

    	lv_label_set_text_fmt(button_icon_right, "%c", rssi_icon);
    }
}

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

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/
static void wifi_list_event_handler(lv_event_t * e){

	WifiCreds_t *creds = 0;
	int a;
	const char *ssid = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {

//    	SPIFFS_IsPasswordSaved((char *)lv_list_get_btn_text(wifi_list, obj));

    	ssid = lv_list_get_btn_text(wifi_list, obj);

    	// prepare return data
    	creds = calloc(1, sizeof(WifiCreds_t));
    	if(0 == creds) goto error;
    	a = strnlen(ssid, 33);
    	if(33 == a) goto error;
    	creds->ssid = malloc(a + 1);
    	if(0 == creds->ssid) goto error;
    	memcpy(creds->ssid, ssid, a + 1);

    	Wifi_Connect(creds);
    }

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

/* create base of wifi popup object */
static void wifi_popup_create_panel(void){

	top_background = lv_obj_create(lv_layer_top());
	lv_obj_add_style(top_background, &style_popup, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_size(top_background, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_bg_opa(top_background, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(top_background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	panel = lv_obj_create(top_background);
	lv_obj_add_style(panel, &style_popup, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_size(panel, 200, 150);
	lv_obj_set_align(panel, LV_ALIGN_CENTER);

	top_text = lv_label_create(panel);
	lv_obj_add_style(top_text, &style_popup, LV_PART_MAIN | LV_STATE_DEFAULT);
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
	lv_obj_add_style(wifi_popup_line, &style_popup, LV_PART_MAIN | LV_STATE_DEFAULT);
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

/**************************************************************
 * Animation helpers
 ***************************************************************/

/* animate line opacity */
static void set_line_opa(void *obj, int32_t val){

	lv_obj_set_style_line_opa(obj, val, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* callback when animation of wifi popup "sign" is done */
static void wifi_popup_line_animation_ready(struct _lv_anim_t *a){

	lv_anim_del(wifi_popup_line, NULL);
	lv_obj_del_delayed(top_background, 500);
}
