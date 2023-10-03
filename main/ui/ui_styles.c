#include "ui_styles.h"
#include "ui.h"

ThemeColorsSet_t UI_CurrentTheme;

lv_style_t 	UI_ScreenStyle, UI_ButtonStyle,
			UI_Icon24Style,

			UI_PopupPanelStyle,
			UI_ClockLabelStyle, UI_LabelIcon36DarkStyle,
			UI_Label30ContrastStyle, UI_Label30DarkStyle,
			UI_Label14ContrastStyle,
			UI_Label16DarkUnderlineStyle, UI_Label16ContrastStyle,
			UI_ArcRSSIStyle, UI_HorizontalLineStyle,
			UI_StartupPanelStyle, UI_Varino30BackgroundLabelStyle, UI_Varino18DarkLabelStyle,
			UI_Varino12ContrastLabelStyle;

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

	lv_style_init(&UI_PopupPanelStyle);
	lv_style_set_bg_color(&UI_PopupPanelStyle, UI_CurrentTheme.background_color_base);
	lv_style_set_bg_opa(&UI_PopupPanelStyle, LV_OPA_COVER);
	lv_style_set_border_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_border_opa(&UI_PopupPanelStyle, LV_OPA_COVER);
	lv_style_set_text_font(&UI_PopupPanelStyle, &lv_font_montserrat_16);
	lv_style_set_text_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_line_color(&UI_PopupPanelStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_line_width(&UI_PopupPanelStyle, 8);
	lv_style_set_line_rounded(&UI_PopupPanelStyle, true);

	lv_style_init(&UI_ScreenStyle);
	lv_style_set_bg_color(&UI_ScreenStyle, UI_CurrentTheme.background_color_base);
	lv_style_set_bg_opa(&UI_ScreenStyle, LV_OPA_COVER);

	lv_style_init(&UI_ClockLabelStyle);
	lv_style_set_bg_opa(&UI_ClockLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_ClockLabelStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_ClockLabelStyle, &ui_font_digital144);
	lv_style_set_text_opa(&UI_ClockLabelStyle, LV_OPA_COVER);

	lv_style_init(&UI_LabelIcon36DarkStyle);
	lv_style_set_bg_opa(&UI_LabelIcon36DarkStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_LabelIcon36DarkStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_LabelIcon36DarkStyle, &ui_font_UIIconsNew36);
	lv_style_set_text_opa(&UI_LabelIcon36DarkStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label30ContrastStyle);
	lv_style_set_bg_opa(&UI_Label30ContrastStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label30ContrastStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Label30ContrastStyle, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Label30ContrastStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label14ContrastStyle);
	lv_style_set_bg_opa(&UI_Label14ContrastStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label14ContrastStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Label14ContrastStyle, &lv_font_montserrat_14);
	lv_style_set_text_opa(&UI_Label14ContrastStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label16ContrastStyle);
	lv_style_set_bg_opa(&UI_Label16ContrastStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label16ContrastStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Label16ContrastStyle, &lv_font_montserrat_16);
	lv_style_set_text_align(&UI_Label16ContrastStyle, LV_TEXT_ALIGN_CENTER);
	lv_style_set_text_opa(&UI_Label16ContrastStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label30DarkStyle);
	lv_style_set_bg_opa(&UI_Label30DarkStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label30DarkStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Label30DarkStyle, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Label30DarkStyle, LV_OPA_COVER);

	lv_style_init(&UI_Label16DarkUnderlineStyle);
	lv_style_set_bg_opa(&UI_Label16DarkUnderlineStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Label16DarkUnderlineStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Label16DarkUnderlineStyle, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Label16DarkUnderlineStyle, LV_OPA_COVER);
	lv_style_set_text_decor(&UI_Label16DarkUnderlineStyle, LV_TEXT_DECOR_UNDERLINE);

	lv_style_init(&UI_ArcRSSIStyle);
	lv_style_set_width(&UI_ArcRSSIStyle, 100);
	lv_style_set_height(&UI_ArcRSSIStyle, 100);
	lv_style_set_bg_opa(&UI_ArcRSSIStyle, LV_OPA_TRANSP);
	lv_style_set_arc_color(&UI_ArcRSSIStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_arc_opa(&UI_ArcRSSIStyle, LV_OPA_30);
	lv_style_set_arc_width(&UI_ArcRSSIStyle, 5);

	lv_style_init(&UI_HorizontalLineStyle);
	lv_style_set_width(&UI_HorizontalLineStyle, 320);
	lv_style_set_height(&UI_HorizontalLineStyle, 2);
	lv_style_set_bg_opa(&UI_HorizontalLineStyle, LV_OPA_COVER);
	lv_style_set_bg_color(&UI_HorizontalLineStyle, UI_CurrentTheme.contrast_color);

	lv_style_init(&UI_StartupPanelStyle);
	lv_style_set_width(&UI_StartupPanelStyle, 280);
	lv_style_set_height(&UI_StartupPanelStyle, 100);
	lv_style_set_bg_opa(&UI_StartupPanelStyle, LV_OPA_TRANSP);
	lv_style_set_bg_color(&UI_StartupPanelStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_border_width(&UI_StartupPanelStyle, 0);
	lv_style_set_shadow_color(&UI_StartupPanelStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_shadow_opa(&UI_StartupPanelStyle, LV_OPA_TRANSP);
	lv_style_set_shadow_width(&UI_StartupPanelStyle, 15);
	lv_style_set_shadow_spread(&UI_StartupPanelStyle, 5);

	lv_style_init(&UI_Varino30BackgroundLabelStyle);
	lv_style_set_bg_opa(&UI_Varino30BackgroundLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Varino30BackgroundLabelStyle, UI_CurrentTheme.background_color_base);
	lv_style_set_text_font(&UI_Varino30BackgroundLabelStyle, &ui_font_Varino30);
	lv_style_set_text_opa(&UI_Varino30BackgroundLabelStyle, LV_OPA_COVER);

	lv_style_init(&UI_Varino18DarkLabelStyle);
	lv_style_set_bg_opa(&UI_Varino18DarkLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Varino18DarkLabelStyle, UI_CurrentTheme.main_color_ext);
	lv_style_set_text_font(&UI_Varino18DarkLabelStyle, &ui_font_Varino18);
	lv_style_set_text_opa(&UI_Varino18DarkLabelStyle, LV_OPA_COVER);

	lv_style_init(&UI_Varino12ContrastLabelStyle);
	lv_style_set_bg_opa(&UI_Varino12ContrastLabelStyle, LV_OPA_TRANSP);
	lv_style_set_text_color(&UI_Varino12ContrastLabelStyle, UI_CurrentTheme.contrast_color);
	lv_style_set_text_font(&UI_Varino12ContrastLabelStyle, &ui_font_Varino12);
	lv_style_set_text_opa(&UI_Varino12ContrastLabelStyle, LV_OPA_COVER);

//	// init list style
//	lv_style_init(&style_list);
//	lv_style_set_bg_opa(&style_list, LV_OPA_TRANSP);
//	lv_style_set_border_opa(&style_list, LV_OPA_TRANSP);
//	lv_style_set_text_font(&style_list, &lv_font_montserrat_16);
//	lv_style_set_text_color(&style_list, lv_color_hex(0xF2921D));
}
