// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: ONLINE_TABLE_CLOCK

#ifndef _ONLINE_TABLE_CLOCK_UI_H
#define _ONLINE_TABLE_CLOCK_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"
// SCREEN: ui_StartupScreen
void ui_StartupScreen_screen_init(void);
extern lv_obj_t * ui_StartupScreen;
extern lv_obj_t * ui_StartupScreenPanel;
extern lv_obj_t * ui_OTCPROJECTLabel;
extern lv_obj_t * ui_OnlineTableClockLabel;
extern lv_obj_t * ui_ByJuraszekLLabel;
extern lv_obj_t * ui_LogosImage;
extern lv_obj_t * ui_VersionLabel;

extern lv_obj_t * ui____initial_actions0;

LV_IMG_DECLARE(ui_img_logos_png);    // assets/Logos.png

LV_FONT_DECLARE(ui_font_UIIconsNew16);
LV_FONT_DECLARE(ui_font_UIIconsNew36);
LV_FONT_DECLARE(ui_font_Varino12);
LV_FONT_DECLARE(ui_font_Varino18);
LV_FONT_DECLARE(ui_font_Varino30);
LV_FONT_DECLARE(ui_font_digital144);

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
