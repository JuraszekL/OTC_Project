#include "ui_styles.h"
#include "ui.h"

ThemeColorsSet_t UI_CurrentTheme;

lv_style_t UI_ButtonStyle, UI_ButtonLabelStyle, UI_PopupPanelStyle, UI_ScreenStyle, UI_ClockLabelStyle,
			UI_Label30ContrastStyle, UI_Label30DarkStyle, UI_Label14ContrastStyle, UI_Label16DarkUnderlineStyle,
			UI_ArcRSSIStyle;

void UI_InitStyles(void){

	UI_CurrentTheme.background_color = lv_color_hex(0x262223);
	UI_CurrentTheme.main_color_bright = lv_color_hex(0xF2921D);
	UI_CurrentTheme.main_color_dark = lv_color_hex(0xF26B1D);
	UI_CurrentTheme.bg_contr_color = lv_color_hex(0xF2F2F2);

	lv_style_init(&UI_ButtonStyle);
	lv_style_set_width(&UI_ButtonStyle, 100);
	lv_style_set_height(&UI_ButtonStyle, 50);
	lv_style_set_bg_opa(&UI_ButtonStyle, LV_OPA_TRANSP);
	lv_style_set_border_color(&UI_ButtonStyle, UI_CurrentTheme.main_color_bright);
	lv_style_set_border_opa(&UI_ButtonStyle, LV_OPA_COVER);
	lv_style_set_border_width(&UI_ButtonStyle, 4);
	lv_style_set_shadow_opa(&UI_ButtonStyle, LV_OPA_TRANSP);

	lv_style_init(&UI_ButtonLabelStyle);
	lv_style_set_width(&UI_ButtonLabelStyle, LV_SIZE_CONTENT);
	lv_style_set_align(&UI_ButtonLabelStyle, LV_ALIGN_CENTER);
	lv_style_set_height(&UI_ButtonLabelStyle, LV_SIZE_CONTENT);
	lv_style_set_bg_opa(&UI_ButtonLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_ButtonLabelStyle, UI_CurrentTheme.main_color_bright);
	lv_style_set_text_font(&UI_ButtonLabelStyle, &ui_font_UIIconsNew36);
	lv_style_set_text_opa(&UI_ButtonLabelStyle, LV_OPA_COVER);

	lv_style_init(&UI_PopupPanelStyle);
	lv_style_set_bg_color(&UI_PopupPanelStyle, UI_CurrentTheme.background_color);
	lv_style_set_bg_opa(&UI_PopupPanelStyle, LV_OPA_COVER);
	lv_style_set_border_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_dark);
	lv_style_set_border_opa(&UI_PopupPanelStyle, LV_OPA_COVER);
	lv_style_set_text_font(&UI_PopupPanelStyle, &lv_font_montserrat_16);
	lv_style_set_text_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_bright);
	lv_style_set_line_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_bright);
	lv_style_set_line_width(&UI_PopupPanelStyle, 8);
	lv_style_set_line_rounded(&UI_PopupPanelStyle, true);

	lv_style_init(&UI_ScreenStyle);
	lv_style_set_bg_color(&UI_ScreenStyle, UI_CurrentTheme.background_color);
	lv_style_set_bg_opa(&UI_ScreenStyle, LV_OPA_COVER);

	lv_style_init(&UI_ClockLabelStyle);
	lv_style_set_bg_opa(&UI_ClockLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_ClockLabelStyle, UI_CurrentTheme.main_color_dark);
	lv_style_set_text_font(&UI_ClockLabelStyle, &ui_font_digital144);
	lv_style_set_text_opa(&UI_ClockLabelStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label30ContrastStyle);
	lv_style_set_bg_opa(&UI_Label30ContrastStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label30ContrastStyle, UI_CurrentTheme.bg_contr_color);
	lv_style_set_text_font(&UI_Label30ContrastStyle, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Label30ContrastStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label14ContrastStyle);
	lv_style_set_bg_opa(&UI_Label14ContrastStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label14ContrastStyle, UI_CurrentTheme.bg_contr_color);
	lv_style_set_text_font(&UI_Label14ContrastStyle, &lv_font_montserrat_14);
	lv_style_set_text_opa(&UI_Label14ContrastStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label30DarkStyle);
	lv_style_set_bg_opa(&UI_Label30DarkStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label30DarkStyle, UI_CurrentTheme.main_color_dark);
	lv_style_set_text_font(&UI_Label30DarkStyle, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Label30DarkStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label16DarkUnderlineStyle);
	lv_style_set_bg_opa(&UI_Label16DarkUnderlineStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label16DarkUnderlineStyle, UI_CurrentTheme.main_color_dark);
	lv_style_set_text_font(&UI_Label16DarkUnderlineStyle, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Label16DarkUnderlineStyle, LV_OPA_COVER);
	lv_style_set_text_decor(&UI_Label16DarkUnderlineStyle, LV_TEXT_DECOR_UNDERLINE);

	lv_style_init(&UI_ArcRSSIStyle);
	lv_style_set_width(&UI_ArcRSSIStyle, 100);
	lv_style_set_height(&UI_ArcRSSIStyle, 100);
	lv_style_set_bg_opa(&UI_ArcRSSIStyle, LV_OPA_TRANSP);
	lv_style_set_arc_color(&UI_ArcRSSIStyle, UI_CurrentTheme.bg_contr_color);
	lv_style_set_arc_opa(&UI_ArcRSSIStyle, LV_OPA_50);
	lv_style_set_arc_width(&UI_ArcRSSIStyle, 5);
}
