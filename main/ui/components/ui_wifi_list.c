#include "ui.h"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
struct list_objects {

	lv_obj_t *obj;
	struct list_objects *next;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void wifi_list_event_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *wifi_list;
static struct list_objects *wifi_list_objects;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize wifi list component */
void UI_WifiListInit(lv_obj_t *screen){

	if(true == lv_obj_is_valid(wifi_list)) return;	//skip if already initialized

	// init list
	wifi_list = lv_list_create(screen);
    lv_obj_set_size(wifi_list, 320, 250);
    lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 145);
    lv_obj_set_style_bg_opa(wifi_list, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(wifi_list, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
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

/* add single object to list */
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
	lv_obj_add_event_cb(new_obj->obj, wifi_list_event_handler, LV_EVENT_SHORT_CLICKED, NULL);
	lv_obj_add_event_cb(new_obj->obj, wifi_list_event_handler, LV_EVENT_LONG_PRESSED, NULL);
	lv_obj_add_style(new_obj->obj, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(new_obj->obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(new_obj->obj, UI_CurrentTheme.main_color_ext, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(new_obj->obj, UI_CurrentTheme.background_color_ext, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(new_obj->obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_radius(new_obj->obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

	// add right aligned label to the button
	button_icon_right = lv_label_create(new_obj->obj);
    lv_obj_set_width(button_icon_right, LV_SIZE_CONTENT);
    lv_obj_set_height(button_icon_right, LV_SIZE_CONTENT);
    lv_obj_set_align(button_icon_right, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_align(button_icon_right, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(button_icon_right, &UI_Icon16Style, LV_PART_MAIN | LV_STATE_DEFAULT);

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

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/
static void wifi_list_event_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    const char *ssid = lv_list_get_btn_text(wifi_list, obj);

    if(code == LV_EVENT_SHORT_CLICKED) {

    	SPIFFS_GetPassAndConnect((char *)ssid);
    }
    else if(code == LV_EVENT_LONG_PRESSED) {

    	SPIFFS_DeletePass((char *)ssid);
    }
}

