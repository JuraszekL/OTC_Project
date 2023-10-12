#include "ui.h"

#include "components/ui_wifi_list.h"
#include "components/ui_wifi_popup.h"

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
static lv_obj_t *ui_WifiScreen, *ui_WifiScreenBackButton, *ui_WifiScreenRSSIArc, *ui_WifiScreenRSSIValueLabel,
			*ui_WifiScreenRSSIdBmLabel, *ui_WifiScreenSSIDLabel, *ui_WifiScreenAPDetailsLabel, *ui_WifiScreenTopPanel;

extern const char *Authentication_Modes[];


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize wifi screen */
void UI_WifiScreen_Init(void){

	UI_ScreenCreate(&ui_WifiScreen);

	UI_BackButtonCreate(&ui_WifiScreen, &ui_WifiScreenBackButton);
	lv_obj_add_event_cb(ui_WifiScreenBackButton, ui_wifi_screen_evt_handler, LV_EVENT_RELEASED, NULL);

	ui_WifiScreenTopPanel = lv_obj_create(ui_WifiScreen);
	lv_obj_add_style(ui_WifiScreenTopPanel, &UI_PanelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_align(ui_WifiScreenTopPanel, LV_ALIGN_TOP_MID);
    lv_obj_set_width(ui_WifiScreenTopPanel, 310);
    lv_obj_set_height(ui_WifiScreenTopPanel, 135);
    lv_obj_set_y(ui_WifiScreenTopPanel, 5);
    lv_obj_clear_flag(ui_WifiScreenTopPanel, LV_OBJ_FLAG_SCROLLABLE);

    ui_WifiScreenRSSIArc = lv_arc_create(ui_WifiScreenTopPanel);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_WifiScreenRSSIArc, &UI_ArcRSSIStyle, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_WifiScreenRSSIArc, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(ui_WifiScreenRSSIArc, -10);
    lv_arc_set_range(ui_WifiScreenRSSIArc, -120, -30);
    lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
    lv_arc_set_bg_angles(ui_WifiScreenRSSIArc, 0, 360);
    lv_arc_set_rotation(ui_WifiScreenRSSIArc, 270);
    lv_obj_clear_flag(ui_WifiScreenRSSIArc, LV_OBJ_FLAG_CLICKABLE);      /// Flags

    ui_WifiScreenRSSIValueLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_add_style(ui_WifiScreenRSSIValueLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIValueLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIValueLabel, -5);
    lv_obj_set_align(ui_WifiScreenRSSIValueLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIValueLabel, "---");

    ui_WifiScreenRSSIdBmLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_add_style(ui_WifiScreenRSSIdBmLabel, &UI_Text14Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIdBmLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIdBmLabel, 20);
    lv_obj_set_align(ui_WifiScreenRSSIdBmLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIdBmLabel, "dBm");

    ui_WifiScreenSSIDLabel = lv_label_create(ui_WifiScreenTopPanel);
    lv_obj_add_style(ui_WifiScreenSSIDLabel, &UI_Text16UnderlineStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_WifiScreenSSIDLabel, LV_ALIGN_TOP_LEFT);
    lv_obj_set_width(ui_WifiScreenSSIDLabel, 170);
    lv_obj_set_height(ui_WifiScreenSSIDLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenSSIDLabel, 15);
    lv_obj_set_y(ui_WifiScreenSSIDLabel, 25);
    lv_label_set_long_mode(ui_WifiScreenSSIDLabel, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(ui_WifiScreenSSIDLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_WifiScreenSSIDLabel, "");

    ui_WifiScreenAPDetailsLabel = lv_label_create(ui_WifiScreenTopPanel);
    lv_obj_add_style(ui_WifiScreenAPDetailsLabel, &UI_Text14Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_WifiScreenAPDetailsLabel, LV_ALIGN_TOP_LEFT);
    lv_obj_set_width(ui_WifiScreenAPDetailsLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenAPDetailsLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenAPDetailsLabel, 15);
    lv_obj_set_y(ui_WifiScreenAPDetailsLabel, 55);
    lv_label_set_text(ui_WifiScreenAPDetailsLabel, "");

    UI_WifiListInit(ui_WifiScreen);
    UI_WifiPopup_MutexInit();
}

/* load wifi screen */
void UI_WifiScreen_Load(uint32_t delay){

	lv_scr_load_anim(ui_WifiScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, delay, false);
}

/* set/clear details about connected AP on WifiScreen */
void UI_WifiScreen_SetApDetails(UI_DetailedAPData_t *data){

	const char *mode_str = 0;
	uint8_t r, g, b = 0, diff;
	lv_style_value_t value;

	if(0 != data){

		// get pointer to authentication mode string
		mode_str = Authentication_Modes[data->mode];

		lv_arc_set_value(ui_WifiScreenRSSIArc, data->rssi);
		lv_label_set_text_fmt(ui_WifiScreenRSSIValueLabel, "%d", data->rssi);
		lv_label_set_text(ui_WifiScreenSSIDLabel, data->ssid);
		lv_label_set_text_fmt(ui_WifiScreenAPDetailsLabel, "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n"
				"IPv4: %s\n%s", data->mac[0], data->mac[1], data->mac[2], data->mac[3], data->mac[4],
				data->mac[5], (char *)data->ip, mode_str);

		/* calculate RSSI arc color
		 * <---------------------------------------------------------------------------->
		 * -30						-60						-90						-120	dBm
		 * 0 ---------------------> 255						255						255		R
		 * 255						255 <-------------------0						0		G
		 * 0						0						0						0		B
		 *
		 * RSSI arc has pure green colour if signal strength is -30dBm, then shifts to yellow, and below
		 * -60dBm moves into red reaching pure red at -90dBm.
		 * */


		if((int)-60 <= data->rssi){

			g = 255;
			diff = abs((int)-30 - data->rssi);
			r = ((unsigned int)(diff * 255))/(unsigned int)30;
		}
		else if(((int)-60 > data->rssi) && ((int)-90 <= data->rssi)){

			r = 255;
			diff = abs((int)-90 - data->rssi);
			g = ((unsigned int)(diff * 255))/(unsigned int)30;
		}
		else{

			r = 255;
			g = 0;
		}

		lv_obj_set_style_arc_color(ui_WifiScreenRSSIArc, lv_color_make(r, g, b), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_arc_color(ui_WifiScreenRSSIArc, lv_color_make(r, g, b), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	}
	else{

		// set default values for AP info
		lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
		lv_label_set_text(ui_WifiScreenRSSIValueLabel, "---");
		lv_label_set_text(ui_WifiScreenSSIDLabel, "");
		lv_label_set_text(ui_WifiScreenAPDetailsLabel, "");
		if(LV_RES_OK == lv_style_get_prop(&UI_ArcRSSIStyle, LV_STYLE_ARC_COLOR, &value)){

			lv_obj_set_style_arc_color(ui_WifiScreenRSSIArc, value.color, LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_obj_set_style_arc_color(ui_WifiScreenRSSIArc, value.color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
	}
}

/* run popup when wifi is connecting */
void UI_WifiScreen_PopupConnecting(WifiCreds_t *creds){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_Connecting(creds->ssid);
	}
}

/* run popup when wifi is connected */
void UI_WifiScreen_PopupConnected(UI_DetailedAPData_t *data){

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
void UI_WifiScreen_PopupConnectError(void){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_NotConnected();
	}
}

/* run popup when wifi password is required */
void UI_WifiScreen_PopupGetPass(WifiCreds_t *creds){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_GetPass(creds);
	}
}

void UI_WifiScreen_PopupPassDeleted(char *ssid){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_PassDeleted(ssid);
	}
}

void UI_WifiScreen_PopupPassNotDeleted(char *ssid){

	// if wifi screen is actual create popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_PassNotDeleted(ssid);
	}
}

/* delete wifi popup */
void UI_WifiScreen_PopupDelete(void){

	// if wifi screen is actual delete popup
	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiPopup_Delete();
	}
}

/* add found AP to wifi list */
void UI_WifiScreen_WifiListAdd(UI_BasicAPData_t *data){

	if(ui_WifiScreen == lv_scr_act()){

		UI_WifiListAdd(data->is_protected, data->ssid, data->rssi);
	}
}

/* clear list with found AP's */
void UI_WifiScreen_WifiListClear(void){

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

    if((event_code == LV_EVENT_RELEASED) && (target == ui_WifiScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_WIFISCR_BACK_BTN_CLICKED, 0);
    }
}
