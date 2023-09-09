// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: ONLINE_TABLE_CLOCK

#include "ui.h"
#include "ui_helpers.h"
#include "ui_styles.h"

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
//    ui_WeatherDetailsScrren_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_StartupScreen);
}

void UI_ScreenCreate(lv_obj_t **screen){

    *screen = lv_obj_create(NULL);
    lv_obj_add_style(*screen, &UI_ScreenStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(*screen, LV_OBJ_FLAG_SCROLLABLE);
}

void UI_BackButtonCreate(lv_obj_t **screen, lv_obj_t **button){

	lv_obj_t *back_but_label;

	*button = lv_btn_create(*screen);
    lv_obj_add_style(*button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(*button, 20);
    lv_obj_set_y(*button, -20);
    lv_obj_set_align(*button, LV_ALIGN_BOTTOM_LEFT);

    back_but_label = lv_label_create(*button);
    lv_obj_add_style(back_but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(back_but_label, "%c", ICON_LEFT_ARROW);
}

void UI_HorizontalLineCreate(lv_obj_t **screen, lv_obj_t **line){

	*line = lv_obj_create(*screen);
	lv_obj_add_style(*line, &UI_HorizontalLineStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
}
