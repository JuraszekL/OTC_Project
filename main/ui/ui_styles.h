#ifndef MAIN_UI_UI_STYLES_H_
#define MAIN_UI_UI_STYLES_H_

#include "lvgl.h"

typedef struct {

	lv_color_t background_color_base;
	lv_color_t background_color_ext;
	lv_color_t main_color_base;
	lv_color_t main_color_ext;
	lv_color_t contrast_color;

} ThemeColorsSet_t;

extern ThemeColorsSet_t UI_CurrentTheme;

extern lv_style_t 	UI_ScreenStyle, UI_ButtonStyle, UI_CheckboxStyle,
					UI_Icon24Style, UI_Icon16Style,
					UI_Text30Style, UI_Text16UnderlineStyle, UI_Text16Style, UI_Text14Style,
					UI_ClockStyle,UI_ArcRSSIStyle, UI_PanelStyle;

void UI_InitStyles(void);

#endif /* MAIN_UI_UI_STYLES_H_ */
