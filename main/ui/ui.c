// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.0
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
lv_obj_t * ui____initial_actions0;
const lv_img_dsc_t * ui_imgset_all[4] = {&ui_img_all2_png, &ui_img_all3_png, &ui_img_all4_png, &ui_img_all5_png};

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_StartupScreen_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_StartupScreen);
}
