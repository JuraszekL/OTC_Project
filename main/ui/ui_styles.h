#ifndef MAIN_UI_UI_STYLES_H_
#define MAIN_UI_UI_STYLES_H_

#include "lvgl.h"

/**************************************************************
 * Structure with current theme
 ***************************************************************/
typedef struct {

	lv_color_t background_color_base;
	lv_color_t background_color_ext;
	lv_color_t main_color_base;
	lv_color_t main_color_ext;
	lv_color_t contrast_color;

} ThemeColorsSet_t;

extern ThemeColorsSet_t UI_CurrentTheme;

/**************************************************************
 * Public styles
 ***************************************************************/
extern lv_style_t 	UI_ScreenStyle, UI_ButtonStyle, UI_CheckboxStyle,
					UI_Icon24Style, UI_Icon16Style,
					UI_Text30Style, UI_Text16UnderlineStyle, UI_Text16Style, UI_Text14Style,
					UI_ClockStyle, UI_ArcRSSIStyle, UI_PanelStyle, UI_DropdownStyle,
					UI_SliderMainStyle, UI_SliderIndicatorStyle, UI_SliderKnobStyle;

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_InitStyles(void);
const char ** UI_GetThemesList(uint8_t *themes_number);
void UI_ChangeTheme(char *theme_name);

#endif /* MAIN_UI_UI_STYLES_H_ */
