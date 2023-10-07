#include "ui_styles.h"
#include "ui.h"

ThemeColorsSet_t UI_CurrentTheme;

lv_style_t 	UI_ScreenStyle, UI_ButtonStyle, UI_CheckboxStyle,
			UI_Icon24Style, UI_Icon16Style,
			UI_Text30Style, UI_Text16UnderlineStyle, UI_Text16Style, UI_Text14Style,
			UI_ClockStyle, UI_ArcRSSIStyle, UI_PanelStyle;

void UI_InitStyles(void){

//	UI_CurrentTheme.background_color_base = lv_color_hex(0x262223);
//	UI_CurrentTheme.main_color_base = lv_color_hex(0xF2921D);
//	UI_CurrentTheme.main_color_ext = lv_color_hex(0xF26B1D);
//	UI_CurrentTheme.contrast_color = lv_color_hex(0xF2F2F2);

	UI_CurrentTheme.background_color_base = lv_color_hex(0x000000);
	UI_CurrentTheme.background_color_ext = lv_color_hex(0x292b2c);
	UI_CurrentTheme.main_color_base = lv_color_hex(0x3491ff);
	UI_CurrentTheme.main_color_ext = lv_color_hex(0xf0f0f0);
	UI_CurrentTheme.contrast_color = lv_color_hex(0xeac025);


	lv_style_init(&UI_ScreenStyle);
	lv_style_set_bg_color(&UI_ScreenStyle, UI_CurrentTheme.background_color_base);
	lv_style_set_bg_opa(&UI_ScreenStyle, LV_OPA_COVER);

	lv_style_init(&UI_ButtonStyle);
	lv_style_set_bg_opa(&UI_ButtonStyle, LV_OPA_TRANSP);
	lv_style_set_border_color(&UI_ButtonStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_border_opa(&UI_ButtonStyle, LV_OPA_COVER);
	lv_style_set_border_width(&UI_ButtonStyle, 4);
	lv_style_set_shadow_opa(&UI_ButtonStyle, LV_OPA_TRANSP);

	lv_style_init(&UI_Icon24Style);
	lv_style_set_bg_opa(&UI_Icon24Style, LV_OPA_TRANSP);
	lv_style_set_text_opa(&UI_Icon24Style, LV_OPA_COVER);
	lv_style_set_text_color(&UI_Icon24Style, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Icon24Style, &ui_font_UIIcons24);

	lv_style_init(&UI_Icon16Style);
	lv_style_set_bg_opa(&UI_Icon16Style, LV_OPA_TRANSP);
	lv_style_set_text_opa(&UI_Icon16Style, LV_OPA_COVER);
	lv_style_set_text_color(&UI_Icon16Style, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Icon16Style, &ui_font_UIIconsNew16);

	lv_style_init(&UI_ClockStyle);
	lv_style_set_bg_opa(&UI_ClockStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_ClockStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_text_font(&UI_ClockStyle, &ui_font_digital144);
	lv_style_set_text_opa(&UI_ClockStyle, LV_OPA_COVER);

	lv_style_init(&UI_Text30Style);
	lv_style_set_bg_opa(&UI_Text30Style, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Text30Style, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Text30Style, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Text30Style, LV_OPA_COVER);

	lv_style_init(&UI_Text14Style);
	lv_style_set_bg_opa(&UI_Text14Style, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Text14Style, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Text14Style, &lv_font_montserrat_14);
	lv_style_set_text_opa(&UI_Text14Style, LV_OPA_COVER);

	lv_style_init(&UI_Text16Style);
	lv_style_set_bg_opa(&UI_Text16Style, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Text16Style, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Text16Style, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Text16Style, LV_OPA_COVER);

	lv_style_init(&UI_Text16UnderlineStyle);
	lv_style_set_bg_opa(&UI_Text16UnderlineStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Text16UnderlineStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_text_font(&UI_Text16UnderlineStyle, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Text16UnderlineStyle, LV_OPA_COVER);
	lv_style_set_text_decor(&UI_Text16UnderlineStyle, LV_TEXT_DECOR_UNDERLINE);

	lv_style_init(&UI_ArcRSSIStyle);
	lv_style_set_width(&UI_ArcRSSIStyle, 100);
	lv_style_set_height(&UI_ArcRSSIStyle, 100);
	lv_style_set_bg_opa(&UI_ArcRSSIStyle, LV_OPA_TRANSP);
	lv_style_set_arc_color(&UI_ArcRSSIStyle, UI_CurrentTheme.background_color_ext);
	lv_style_set_arc_opa(&UI_ArcRSSIStyle, LV_OPA_30);
	lv_style_set_arc_width(&UI_ArcRSSIStyle, 5);

	lv_style_init(&UI_CheckboxStyle);
	lv_style_set_bg_opa(&UI_CheckboxStyle, LV_OPA_TRANSP);
	lv_style_set_border_color(&UI_CheckboxStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_text_color(&UI_CheckboxStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_CheckboxStyle, &lv_font_montserrat_16);

	lv_style_init(&UI_PanelStyle);
	lv_style_set_bg_opa(&UI_PanelStyle, LV_OPA_COVER);
	lv_style_set_bg_color(&UI_PanelStyle, UI_CurrentTheme.background_color_ext);
	lv_style_set_border_color(&UI_PanelStyle, UI_CurrentTheme.background_color_ext);
	lv_style_set_radius(&UI_PanelStyle, 8);
	lv_style_set_pad_all(&UI_PanelStyle, 0);
}
