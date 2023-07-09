// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.0
// LVGL version: 8.3.6
// Project name: ONLINE_TABLE_CLOCK

#include "../ui.h"

void ui_MainScreen_screen_init(void)
{
    ui_MainScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_MainScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_MainScreen, lv_color_hex(0x262223), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_MainScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ClockLabel = lv_label_create(ui_MainScreen);
    lv_obj_set_width(ui_ClockLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ClockLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_ClockLabel, 0);
    lv_obj_set_y(ui_ClockLabel, 50);
    lv_obj_set_align(ui_ClockLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_ClockLabel, "22:34");
    lv_obj_set_style_text_color(ui_ClockLabel, lv_color_hex(0xF26B1D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ClockLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ClockLabel, &ui_font_digital144, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_DateLabel = lv_label_create(ui_MainScreen);
    lv_obj_set_width(ui_DateLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_DateLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_DateLabel, 15);
    lv_obj_set_y(ui_DateLabel, -10);
    lv_obj_set_align(ui_DateLabel, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_DateLabel, "Thurstday\n07 Jun");
    lv_obj_set_style_text_color(ui_DateLabel, lv_color_hex(0xF2F2F2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_DateLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_DateLabel, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WeatherIcon = lv_label_create(ui_MainScreen);
    lv_obj_set_width(ui_WeatherIcon, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WeatherIcon, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WeatherIcon, 65);
    lv_obj_set_y(ui_WeatherIcon, 80);
    lv_obj_set_align(ui_WeatherIcon, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WeatherIcon, "");
    lv_obj_set_style_text_color(ui_WeatherIcon, lv_color_hex(0xF2811D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WeatherIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WeatherIcon, &ui_font_weather100, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WiFiIconLabel = lv_label_create(ui_MainScreen);
    lv_obj_set_width(ui_WiFiIconLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_WiFiIconLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_WiFiIconLabel, 8);
    lv_obj_set_y(ui_WiFiIconLabel, 8);
    lv_label_set_text(ui_WiFiIconLabel, "B");
    lv_obj_set_style_text_color(ui_WiFiIconLabel, lv_color_hex(0xF2921D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WiFiIconLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WiFiIconLabel, &ui_font_UIIcons20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Panel2 = lv_obj_create(ui_MainScreen);
    lv_obj_set_width(ui_Panel2, 320);
    lv_obj_set_height(ui_Panel2, 2);
    lv_obj_set_x(ui_Panel2, 0);
    lv_obj_set_y(ui_Panel2, 35);
    lv_obj_set_align(ui_Panel2, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_Panel2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

}