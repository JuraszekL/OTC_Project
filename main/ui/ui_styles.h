#ifndef MAIN_UI_UI_STYLES_H_
#define MAIN_UI_UI_STYLES_H_

#include "lvgl.h"

typedef struct {

	lv_color_t background_color;
	lv_color_t main_color_bright;
	lv_color_t main_color_dark;
	lv_color_t bg_contr_color;

} ThemeColorsSet_t;

extern ThemeColorsSet_t UI_CurrentTheme;

extern lv_style_t UI_ButtonStyle, UI_ButtonLabelStyle, UI_PopupPanelStyle;

void UI_InitStyles(void);

#endif /* MAIN_UI_UI_STYLES_H_ */
