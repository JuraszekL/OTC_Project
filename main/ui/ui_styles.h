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

extern lv_style_t 	UI_ButtonStyle, UI_Icon24Style,
					UI_PopupPanelStyle,
					UI_ScreenStyle,
					UI_ClockLabelStyle, UI_LabelIcon36DarkStyle,
					UI_Label30ContrastStyle, UI_Label30DarkStyle,
					UI_Label14ContrastStyle,
					UI_Label16DarkUnderlineStyle, UI_Label16ContrastStyle,
					UI_ArcRSSIStyle, UI_HorizontalLineStyle,
					UI_StartupPanelStyle, UI_Varino30BackgroundLabelStyle, UI_Varino18DarkLabelStyle,
					UI_Varino12ContrastLabelStyle;

void UI_InitStyles(void);

#endif /* MAIN_UI_UI_STYLES_H_ */
