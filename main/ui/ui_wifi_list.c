#include "ui_wifi_list.h"
#include "esp_log.h"

struct list_objects {

	lv_obj_t *obj;
	struct list_objects *next;
};

static void wifi_list_event_handler(lv_event_t * e);

static lv_style_t style_list;
static lv_obj_t *wifi_list;
static struct list_objects *wifi_list_objects;


void UI_WifiListInit(void){

	if(0 != wifi_list) return;

	lv_style_init(&style_list);
	lv_style_set_bg_opa(&style_list, LV_OPA_TRANSP);
	lv_style_set_border_opa(&style_list, LV_OPA_TRANSP);
	lv_style_set_text_font(&style_list, &lv_font_montserrat_16);
	lv_style_set_text_color(&style_list, lv_color_hex(0xF2921D));

	wifi_list = lv_list_create(ui_WifiScreen);
    lv_obj_set_size(wifi_list, 320, 250);
    lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 145);

    lv_obj_add_style(wifi_list, &style_list, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void UI_WifiListClear(void){

	if(0 == wifi_list_objects) return;

	struct list_objects *current, *next;

	current = wifi_list_objects;

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

	struct list_objects *new_obj;
	lv_obj_t *button_icon_right;
	char rssi_icon;

	if(0 == wifi_list_objects){

		wifi_list_objects = calloc(1, sizeof(struct list_objects));
		if(0 == wifi_list_objects) return;
		new_obj = wifi_list_objects;
	}
	else{

		new_obj = wifi_list_objects;
		while(0 != new_obj->next){

			new_obj = new_obj->next;

		}

		new_obj->next = calloc(1, sizeof(struct list_objects));
		if(0 == wifi_list_objects) return;
		new_obj = new_obj->next;
	}

	new_obj->obj = lv_list_add_btn(wifi_list, 0, name);
	lv_obj_add_event_cb(new_obj->obj, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(new_obj->obj, &style_list, LV_PART_MAIN | LV_STATE_DEFAULT);

	button_icon_right = lv_label_create(new_obj->obj);
    lv_obj_set_width(button_icon_right, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(button_icon_right, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(button_icon_right, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_align(button_icon_right, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button_icon_right, &ui_font_UIIconsNew16, LV_PART_MAIN | LV_STATE_DEFAULT);

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

    if(true == is_protected){

    	lv_label_set_text_fmt(button_icon_right, "%c %c",ICON_PADLOCK, rssi_icon);
    }
    else{

    	lv_label_set_text_fmt(button_icon_right, "%c", rssi_icon);
    }

}


static void wifi_list_event_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI("", "Clicked: %s", lv_list_get_btn_text(wifi_list, obj));
        UI_WifiListClear();
    }
}
