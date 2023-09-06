// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: ONLINE_TABLE_CLOCK

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////

// SCREEN: ui_StartupScreen
void ui_StartupScreen_screen_init(void);
lv_obj_t * ui_StartupScreen;
lv_obj_t * ui_StartupScreenPanel;
lv_obj_t * ui_OTCPROJECTLabel;
lv_obj_t * ui_OnlineTableClockLabel;
lv_obj_t * ui_ByJuraszekLLabel;
lv_obj_t * ui_LogosImage;
lv_obj_t * ui_VersionLabel;

// SCREEN: ui_WeatherDetailsScrren
void ui_WeatherDetailsScrren_screen_init(void);
lv_obj_t * ui_WeatherDetailsScrren;
void ui_event_WeatherScreenBackButton(lv_event_t * e);
lv_obj_t * ui_WeatherScreenBackButton;
lv_obj_t * ui_BackArrowIconLabel;
lv_obj_t * ui_WeatherScreenCity;
lv_obj_t * ui_WeatherScreenCountry;
lv_obj_t * ui_WeatherScreenIcon;
lv_obj_t * ui_WeatherScreenTempArc;
lv_obj_t * ui_WeatherScreenTemp;
lv_obj_t * ui_WeatherScreenTempMinMax;
lv_obj_t * ui_WeatherScreenSunriseIconLabel;
lv_obj_t * ui_WeatherScreenRainIconLabel;
lv_obj_t * ui_WeatherScreenSnowIconLabel;
lv_obj_t * ui_WeatherScreenWindIconLabel;
lv_obj_t * ui_WeatherScreenSunriseLabel;
lv_obj_t * ui_WeatherScreenRainLabel;
lv_obj_t * ui_WeatherScreenWindLabel;
lv_obj_t * ui_WeatherScreenSnowLabel;

lv_obj_t * ui____initial_actions0;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_WeatherScreenBackButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
//    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        WetaherScreen_BackButtonClicked(e);
    }
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_StartupScreen_screen_init();
    ui_WeatherDetailsScrren_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_StartupScreen);
}
