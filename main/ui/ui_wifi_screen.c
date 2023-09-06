#include "ui_wifi_screen.h"
#include "ui_wifi_popup.h"
#include "ui_wifi_list.h"
#include "ui_styles.h"
#include "ui.h"
#include "ui_task.h"
#include "lvgl.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_wifi_screen_evt_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
lv_obj_t *ui_WifiScreen, *ui_WifiScreenBackButton, *ui_WifiScreenRSSIArc, *ui_WifiScreenRSSIValueLabel,
			*ui_WifiScreenRSSIdBmLabel, *ui_WifiScreenSSIDLabel, *ui_WifiScreenWifiDetails, *ui_WhiteLineTop1;

extern const char *Authentication_Modes[];


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize wifi screen */
void UI_WifiScreen_Init(void){

	lv_obj_t *wifi_but_label;

    ui_WifiScreen = lv_obj_create(NULL);
    lv_obj_add_style(ui_WifiScreen, &UI_ScreenStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_WifiScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_WifiScreenBackButton = lv_btn_create(ui_WifiScreen);
    lv_obj_add_style(ui_WifiScreenBackButton, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WifiScreenBackButton, 20);
    lv_obj_set_y(ui_WifiScreenBackButton, -20);
    lv_obj_set_align(ui_WifiScreenBackButton, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_event_cb(ui_WifiScreenBackButton, ui_wifi_screen_evt_handler, LV_EVENT_ALL, NULL);

    wifi_but_label = lv_label_create(ui_WifiScreenBackButton);
    lv_obj_add_style(wifi_but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(wifi_but_label, "%c", ICON_LEFT_ARROW);

    ui_WifiScreenRSSIArc = lv_arc_create(ui_WifiScreen);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WifiScreenRSSIArc, -20);
    lv_obj_set_y(ui_WifiScreenRSSIArc, 20);
    lv_obj_set_align(ui_WifiScreenRSSIArc, LV_ALIGN_TOP_RIGHT);
    lv_obj_clear_flag(ui_WifiScreenRSSIArc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_range(ui_WifiScreenRSSIArc, -120, -30);
    lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
    lv_arc_set_bg_angles(ui_WifiScreenRSSIArc, 0, 360);
    lv_arc_set_rotation(ui_WifiScreenRSSIArc, 270);

    ui_WifiScreenRSSIValueLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_add_style(ui_WifiScreenRSSIValueLabel, &UI_Label30ContrastStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIValueLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIValueLabel, -5);
    lv_obj_set_align(ui_WifiScreenRSSIValueLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIValueLabel, "---");

    ui_WifiScreenRSSIdBmLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_add_style(ui_WifiScreenRSSIdBmLabel, &UI_Label14ContrastStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIdBmLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIdBmLabel, 20);
    lv_obj_set_align(ui_WifiScreenRSSIdBmLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIdBmLabel, "dBm");

    ui_WifiScreenSSIDLabel = lv_label_create(ui_WifiScreen);
    lv_obj_add_style(ui_WifiScreenSSIDLabel, &UI_Label16DarkUnderlineStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenSSIDLabel, 170);
    lv_obj_set_height(ui_WifiScreenSSIDLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenSSIDLabel, 20);
    lv_obj_set_y(ui_WifiScreenSSIDLabel, 30);
    lv_label_set_long_mode(ui_WifiScreenSSIDLabel, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(ui_WifiScreenSSIDLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_WifiScreenSSIDLabel, "");

    ui_WifiScreenWifiDetails = lv_label_create(ui_WifiScreen);
    lv_obj_add_style(ui_WifiScreenWifiDetails, &UI_Label14ContrastStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenWifiDetails, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenWifiDetails, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenWifiDetails, 20);
    lv_obj_set_y(ui_WifiScreenWifiDetails, 60);
    lv_label_set_text(ui_WifiScreenWifiDetails, "");

    //TODO zmienić
    ui_WhiteLineTop1 = lv_obj_create(ui_WifiScreen);
    lv_obj_set_width(ui_WhiteLineTop1, 320);
    lv_obj_set_height(ui_WhiteLineTop1, 2);
    lv_obj_set_x(ui_WhiteLineTop1, 0);
    lv_obj_set_y(ui_WhiteLineTop1, 140);
    lv_obj_set_align(ui_WhiteLineTop1, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_WhiteLineTop1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    UI_WifiListInit(ui_WifiScreen);
}

/* load wifi screen */
void UI_WifiScreen_Load(uint32_t delay){

	lv_scr_load_anim(ui_WifiScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, delay, false);
}

/* run popup when wifi is connecting */
void UI_WifiScreen_Connecting(WifiCreds_t *creds){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_Connecting(creds->ssid);
	}
}

/* run popup when wifi is connected */
void UI_WifiScreen_Connected(UI_DetailedAPData_t *data){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		if(0 != data){

			UI_WifiPopup_Connected(data->ssid);
		}
		else{

			UI_WifiPopup_Connected(0);
		}
	}
}

/* run popup when wifi could not be connected */
void UI_WifiScreen_ConnectError(void){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_NotConnected();
	}
}

/* run popup when wifi password is required */
void UI_WifiScreen_GetPass(WifiCreds_t *creds){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_GetPass(creds);
	}
}

/* set/clear details about connected AP on WifiScreen */
void UI_WifiScreen_SetApDetails(UI_DetailedAPData_t *data){

	const char *mode_str = 0;

	if(0 != data){

		// get pointer to authentication mode string
		mode_str = Authentication_Modes[data->mode];

		lv_arc_set_value(ui_WifiScreenRSSIArc, data->rssi);
		lv_label_set_text_fmt(ui_WifiScreenRSSIValueLabel, "%d", data->rssi);
		lv_label_set_text(ui_WifiScreenSSIDLabel, data->ssid);
		lv_label_set_text_fmt(ui_WifiScreenWifiDetails, "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n"
				"IPv4: %s\n%s", data->mac[0], data->mac[1], data->mac[2], data->mac[3], data->mac[4],
				data->mac[5], (char *)data->ip, mode_str);
	}
	else{

		lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
		lv_label_set_text(ui_WifiScreenRSSIValueLabel, "---");
		lv_label_set_text(ui_WifiScreenSSIDLabel, "");
		lv_label_set_text(ui_WifiScreenWifiDetails, "");
	}
}

/* add found AP to wifi list */
void UI_WifiScreen_AddToList(UI_BasicAPData_t *data){

	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiListAdd(data->is_protected, data->ssid, data->rssi);
	}
}

/* clear list with found AP's */
void UI_WifiScreen_ClearList(void){

	UI_WifiListClear();
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* wifi screen events handler */
static void ui_wifi_screen_evt_handler(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if((event_code == LV_EVENT_CLICKED) && (target == ui_WifiScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_WIFISCR_BACK_BTN_CLICKED, 0);
    }
}
