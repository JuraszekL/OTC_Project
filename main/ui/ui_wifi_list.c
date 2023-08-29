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
static lv_style_t style_list;
static lv_obj_t *wifi_list;
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

