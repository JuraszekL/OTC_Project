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
			*ui_WifiScreenRSSIdBmLabel, *ui_WifiScreenSSIDLabel, *ui_WifiScreenAPDetailsLabel, *ui_WifiScreenHorLine;

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
	lv_obj_add_event_cb(ui_WifiScreenBackButton, ui_wifi_screen_evt_handler, LV_EVENT_ALL, NULL);

    UI_HorizontalLineCreate(&ui_WifiScreen, &ui_WifiScreenHorLine);
	lv_obj_set_x(ui_WifiScreenHorLine, 0);
	lv_obj_set_y(ui_WifiScreenHorLine, 140);
	lv_obj_set_align(ui_WifiScreenHorLine, LV_ALIGN_TOP_MID);

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

    ui_WifiScreenAPDetailsLabel = lv_label_create(ui_WifiScreen);
    lv_obj_add_style(ui_WifiScreenAPDetailsLabel, &UI_Label14ContrastStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(ui_WifiScreenAPDetailsLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenAPDetailsLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenAPDetailsLabel, 20);
    lv_obj_set_y(ui_WifiScreenAPDetailsLabel, 60);
    lv_label_set_text(ui_WifiScreenAPDetailsLabel, "");

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

/* perform action when object on wifi list has been clicked */
void UI_WifiScreen_WifiListClicked(lv_obj_t * obj){

	if(0 == obj) return;

	const char *ssid = 0;
	WifiCreds_t *creds = 0;
	int a;

	// get ssid of clicked object
	UI_WifiList_GetClickedSSID(obj, &ssid);
	if(0 == ssid) return;

	// prepare return data
	creds = calloc(1, sizeof(WifiCreds_t));
	if(0 == creds) goto error;
	a = strnlen(ssid, 33);
	if(33 == a) goto error;
	creds->ssid = malloc(a + 1);
	if(0 == creds->ssid) goto error;
	memcpy(creds->ssid, ssid, a + 1);

	ESP_LOGI("ui_wifi_list.c", "calling Wifi_Connect");
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

		UI_WifiList_SetClickable();
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
