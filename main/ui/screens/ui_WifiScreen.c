// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: ONLINE_TABLE_CLOCK

#include "../ui.h"

void ui_WifiScreen_screen_init(void)
{
    ui_WifiScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_WifiScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_WifiScreen, lv_color_hex(0x262223), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_WifiScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WifiScreenBackButton = lv_btn_create(ui_WifiScreen);
    lv_obj_set_width(ui_WifiScreenBackButton, 100);
    lv_obj_set_height(ui_WifiScreenBackButton, 50);
    lv_obj_set_x(ui_WifiScreenBackButton, 20);
    lv_obj_set_y(ui_WifiScreenBackButton, -20);
    lv_obj_set_align(ui_WifiScreenBackButton, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_flag(ui_WifiScreenBackButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_WifiScreenBackButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_WifiScreenBackButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_WifiScreenBackButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_WifiScreenBackButton, lv_color_hex(0xF2921D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_WifiScreenBackButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_WifiScreenBackButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_WifiScreenBackButton, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_WifiScreenBackButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_BackArrowIconLabel2 = lv_label_create(ui_WifiScreenBackButton);
    lv_obj_set_width(ui_BackArrowIconLabel2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_BackArrowIconLabel2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_BackArrowIconLabel2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_BackArrowIconLabel2, "F");
    lv_obj_set_style_text_color(ui_BackArrowIconLabel2, lv_color_hex(0xF2921D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BackArrowIconLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BackArrowIconLabel2, &ui_font_UIIconsNew36, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WifiScreenRSSIArc = lv_arc_create(ui_WifiScreen);
    lv_obj_set_width(ui_WifiScreenRSSIArc, 100);
    lv_obj_set_height(ui_WifiScreenRSSIArc, 100);
    lv_obj_set_x(ui_WifiScreenRSSIArc, -20);
    lv_obj_set_y(ui_WifiScreenRSSIArc, 20);
    lv_obj_set_align(ui_WifiScreenRSSIArc, LV_ALIGN_TOP_RIGHT);
    lv_obj_clear_flag(ui_WifiScreenRSSIArc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_range(ui_WifiScreenRSSIArc, -120, -30);
    lv_arc_set_value(ui_WifiScreenRSSIArc, -120);
    lv_arc_set_bg_angles(ui_WifiScreenRSSIArc, 0, 360);
    lv_arc_set_rotation(ui_WifiScreenRSSIArc, 270);
    lv_obj_set_style_arc_color(ui_WifiScreenRSSIArc, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_WifiScreenRSSIArc, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_WifiScreenRSSIArc, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_width(ui_WifiScreenRSSIArc, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_WifiScreenRSSIArc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_WifiScreenRSSIArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_WifiScreenRSSIValueLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_set_width(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIValueLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIValueLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIValueLabel, -5);
    lv_obj_set_align(ui_WifiScreenRSSIValueLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIValueLabel, "-120");
    lv_obj_set_style_text_color(ui_WifiScreenRSSIValueLabel, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WifiScreenRSSIValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WifiScreenRSSIValueLabel, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WifiScreenRSSIdBmLabel = lv_label_create(ui_WifiScreenRSSIArc);
    lv_obj_set_width(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenRSSIdBmLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenRSSIdBmLabel, 0);
    lv_obj_set_y(ui_WifiScreenRSSIdBmLabel, 20);
    lv_obj_set_align(ui_WifiScreenRSSIdBmLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WifiScreenRSSIdBmLabel, "dBm");
    lv_obj_set_style_text_color(ui_WifiScreenRSSIdBmLabel, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WifiScreenRSSIdBmLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WifiScreenRSSIdBmLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WifiScreenSSIDLabel = lv_label_create(ui_WifiScreen);
    lv_obj_set_width(ui_WifiScreenSSIDLabel, 170);
    lv_obj_set_height(ui_WifiScreenSSIDLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenSSIDLabel, 20);
    lv_obj_set_y(ui_WifiScreenSSIDLabel, 30);
    lv_label_set_long_mode(ui_WifiScreenSSIDLabel, LV_LABEL_LONG_SCROLL);
    lv_label_set_text(ui_WifiScreenSSIDLabel, "FRITZ!Box 7590 FD");
    lv_obj_set_style_text_color(ui_WifiScreenSSIDLabel, lv_color_hex(0xF26B1D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WifiScreenSSIDLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_WifiScreenSSIDLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_decor(ui_WifiScreenSSIDLabel, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WifiScreenSSIDLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WifiScreenWifiDetails = lv_label_create(ui_WifiScreen);
    lv_obj_set_width(ui_WifiScreenWifiDetails, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WifiScreenWifiDetails, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WifiScreenWifiDetails, 20);
    lv_obj_set_y(ui_WifiScreenWifiDetails, 60);
    lv_label_set_text(ui_WifiScreenWifiDetails, "MAC: 14:75:5B:BF:49:79\nIPv4: 192.168.178.30\nWPA2 PSK");
    lv_obj_set_style_text_color(ui_WifiScreenWifiDetails, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WifiScreenWifiDetails, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WifiScreenWifiDetails, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WhiteLineTop1 = lv_obj_create(ui_WifiScreen);
    lv_obj_set_width(ui_WhiteLineTop1, 320);
    lv_obj_set_height(ui_WhiteLineTop1, 2);
    lv_obj_set_x(ui_WhiteLineTop1, 0);
    lv_obj_set_y(ui_WhiteLineTop1, 140);
    lv_obj_set_align(ui_WhiteLineTop1, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_WhiteLineTop1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    lv_obj_add_event_cb(ui_WifiScreenBackButton, ui_event_WifiScreenBackButton, LV_EVENT_ALL, NULL);

}