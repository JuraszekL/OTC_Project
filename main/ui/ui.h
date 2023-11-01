#ifndef _ONLINE_TABLE_CLOCK_UI_H
#define _ONLINE_TABLE_CLOCK_UI_H

#include "lvgl.h"
#include "ui_styles.h"
#include "screens/ui_main_screen.h"
#include "screens/ui_wifi_screen.h"
#include "screens/ui_weather_screen.h"
#include "screens/ui_startup_screen.h"
#include "screens/ui_setup_screen.h"
#include "screens/ui_alarms_screen.h"

/**************************************************************
 * Font icon characters
 ***************************************************************/
#define ICON_WIFI			'A'
#define ICON_NO_WIFI		'B'
#define ICON_SYNC			'C'
#define ICON_NO_SYNC		'D'
#define ICON_UP_ARROW		'E'
#define ICON_LEFT_ARROW		'F'
#define ICON_RIGHT_ARROW	'G'
#define ICON_DOWN_ARROW		'H'
#define ICON_SUNRISE		'I'
#define ICON_RAIN			'J'
#define ICON_WIND			'K'
#define ICON_SNOW			'L'
#define ICON_PADLOCK		'M'
#define ICON_SIGNAL_FULL	'N'
#define ICON_SIGNAL_GOOD	'O'
#define ICON_SIGNAL_MID		'P'
#define ICON_SIGNAL_LOW		'Q'
#define ICON_SETUP			'R'

/**************************************************************
 * Animation time
 ***************************************************************/
#define KEYBOARD_SHOW_HIDE_TIME_MS			300U
#define OK_NOK_LINE_ANIMATION_TIME_MS		2000U

/**************************************************************
 * Structure with basic pointers to popup component
 ***************************************************************/
typedef struct {

	lv_obj_t *background;
	lv_obj_t *panel;
	lv_obj_t *label;

} UI_PopupObj_t;

/**************************************************************
 * Declarations of images and fonts
 ***************************************************************/
LV_IMG_DECLARE(ui_img_logos_png);    // assets/Logos.png

LV_FONT_DECLARE(ui_font_UIIconsNew16);
LV_FONT_DECLARE(ui_font_UIIcons24);
LV_FONT_DECLARE(ui_font_Varino12);
LV_FONT_DECLARE(ui_font_Varino18);
LV_FONT_DECLARE(ui_font_Varino30);
LV_FONT_DECLARE(ui_font_digital144);

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_ScreenCreate(lv_obj_t **screen);
void UI_ButtonCreate(lv_obj_t **parent, lv_obj_t **button, char icon);
void UI_BackButtonCreate(lv_obj_t **parent, lv_obj_t **button);
void UI_CheckboxCreate(lv_obj_t **parent, lv_obj_t **checkbox, char *label);
void UI_PopupCreate(UI_PopupObj_t *popup);
void UI_KeyboardCreate(lv_obj_t **parent, lv_obj_t **keyboard);

#endif
