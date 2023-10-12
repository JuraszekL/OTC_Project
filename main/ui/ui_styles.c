#include "ui_styles.h"
#include "ui.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
struct theme {

	char *theme_name;
	uint32_t color_bckg_base_hex;
	uint32_t color_bckg_ext_hex;
	uint32_t color_main_base_hex;
	uint32_t color_main_ext_hex;
	uint32_t color_contr_hex;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_set_current_theme_colors_to_styles(void);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
ThemeColorsSet_t UI_CurrentTheme;

lv_style_t 	UI_ScreenStyle, UI_ButtonStyle, UI_CheckboxStyle,
			UI_Icon24Style, UI_Icon16Style,
			UI_Text30Style, UI_Text16UnderlineStyle, UI_Text16Style, UI_Text14Style,
			UI_ClockStyle, UI_ArcRSSIStyle, UI_PanelStyle, UI_DropdownStyle;

static const struct theme themes[] = {

		{
				.theme_name = "Orange Dark",
				.color_bckg_base_hex = 0x262223,
				.color_bckg_ext_hex = 0x302d2e,
				.color_main_base_hex = 0xF26B1D,
				.color_main_ext_hex = 0xF2921D,
				.color_contr_hex = 0xF2F2F2,
		},
		{
				.theme_name = "Blue Dark",
				.color_bckg_base_hex = 0x000000,
				.color_bckg_ext_hex = 0x292b2c,
				.color_main_base_hex = 0x3491ff,
				.color_main_ext_hex = 0xf0f0f0,
				.color_contr_hex = 0xeac025,
		},
		{
				.theme_name = "Olive Green",
				.color_bckg_base_hex = 0xc9f0a6,
				.color_bckg_ext_hex = 0xa7e081,
				.color_main_base_hex = 0x3d9900,
				.color_main_ext_hex = 0x348101,
				.color_contr_hex = 0xdf00bc,
		},
		{
				.theme_name = "Flat White",
				.color_bckg_base_hex = 0xd2a271,
				.color_bckg_ext_hex = 0xc6915a,
				.color_main_base_hex = 0x78512c,
				.color_main_ext_hex = 0x543a21,
				.color_contr_hex = 0xd65b00,
		},

		{
				.theme_name = NULL,
		}
};

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize all styles */
void UI_InitStyles(void){

	lv_style_init(&UI_ScreenStyle);
	lv_style_set_bg_opa(&UI_ScreenStyle, LV_OPA_COVER);

	lv_style_init(&UI_ButtonStyle);
	lv_style_set_bg_opa(&UI_ButtonStyle, LV_OPA_TRANSP);
	lv_style_set_border_opa(&UI_ButtonStyle, LV_OPA_COVER);
	lv_style_set_border_width(&UI_ButtonStyle, 4);
	lv_style_set_shadow_opa(&UI_ButtonStyle, LV_OPA_TRANSP);

	lv_style_init(&UI_Icon24Style);
	lv_style_set_bg_opa(&UI_Icon24Style, LV_OPA_TRANSP);
	lv_style_set_text_opa(&UI_Icon24Style, LV_OPA_COVER);
	lv_style_set_text_font(&UI_Icon24Style, &ui_font_UIIcons24);

	lv_style_init(&UI_Icon16Style);
	lv_style_set_bg_opa(&UI_Icon16Style, LV_OPA_TRANSP);
	lv_style_set_text_opa(&UI_Icon16Style, LV_OPA_COVER);
	lv_style_set_text_font(&UI_Icon16Style, &ui_font_UIIconsNew16);

	lv_style_init(&UI_ClockStyle);
	lv_style_set_bg_opa(&UI_ClockStyle, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_ClockStyle, &ui_font_digital144);
	lv_style_set_text_opa(&UI_ClockStyle, LV_OPA_COVER);

	lv_style_init(&UI_Text30Style);
	lv_style_set_bg_opa(&UI_Text30Style, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_Text30Style, &lv_font_montserrat_30);
	lv_style_set_text_opa(&UI_Text30Style, LV_OPA_COVER);

	lv_style_init(&UI_Text14Style);
	lv_style_set_bg_opa(&UI_Text14Style, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_Text14Style, &lv_font_montserrat_14);
	lv_style_set_text_opa(&UI_Text14Style, LV_OPA_COVER);

	lv_style_init(&UI_Text16Style);
	lv_style_set_bg_opa(&UI_Text16Style, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_Text16Style, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Text16Style, LV_OPA_COVER);

	lv_style_init(&UI_Text16UnderlineStyle);
	lv_style_set_bg_opa(&UI_Text16UnderlineStyle, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_Text16UnderlineStyle, &lv_font_montserrat_16);
	lv_style_set_text_opa(&UI_Text16UnderlineStyle, LV_OPA_COVER);
	lv_style_set_text_decor(&UI_Text16UnderlineStyle, LV_TEXT_DECOR_UNDERLINE);

	lv_style_init(&UI_ArcRSSIStyle);
	lv_style_set_width(&UI_ArcRSSIStyle, 100);
	lv_style_set_height(&UI_ArcRSSIStyle, 100);
	lv_style_set_bg_opa(&UI_ArcRSSIStyle, LV_OPA_TRANSP);
	lv_style_set_arc_opa(&UI_ArcRSSIStyle, LV_OPA_30);
	lv_style_set_arc_width(&UI_ArcRSSIStyle, 5);

	lv_style_init(&UI_CheckboxStyle);
	lv_style_set_bg_opa(&UI_CheckboxStyle, LV_OPA_TRANSP);
	lv_style_set_text_font(&UI_CheckboxStyle, &lv_font_montserrat_16);

	lv_style_init(&UI_PanelStyle);
	lv_style_set_bg_opa(&UI_PanelStyle, LV_OPA_COVER);
	lv_style_set_radius(&UI_PanelStyle, 8);
	lv_style_set_pad_all(&UI_PanelStyle, 0);

	lv_style_init(&UI_DropdownStyle);
	lv_style_set_bg_opa(&UI_DropdownStyle, LV_OPA_COVER);
	lv_style_set_text_font(&UI_DropdownStyle, &lv_font_montserrat_16);

	ui_set_current_theme_colors_to_styles();
}

/* returns an array with avalible theme names and number of items */
const char ** UI_GetThemesList(uint8_t *themes_number){

	if(0 == themes_number) return 0;

	const char **theme_list;
	uint8_t a = 0;

	// check number of items within the constant array
	while(NULL != themes[a].theme_name){

		a++;
	}

	// allocate a memory for return data
	theme_list = calloc(a, sizeof(const char *));
	if(0 == theme_list) return 0;

	// return number of elements
	*themes_number = a;

	// copy the names to return array
	while(0 != a){

		a--;
		theme_list[a] = themes[a].theme_name;
	}

	// return the array
	return theme_list;
}

/* changes the theme of UI */
void UI_ChangeTheme(char *theme_name){

	if(0 == theme_name) return;

	uint8_t a = 0;
	int res = 0, len = 0;

	// get the length of theme name
	len = strnlen(theme_name, 32);
	if((0 == len) || (32 == len)) goto cleanup;

	// find the tindex of theme with obtained "theme name"
	while(NULL != themes[a].theme_name){

		res = memcmp(themes[a].theme_name, theme_name, len);
		if(0 == res) break;
		a++;
	}

	// if wrong name has been passed ignore request
	if(NULL == themes[a].theme_name) goto cleanup;

	// set new theme to config
	NVS_SetConfig(CONFIG_THEME, theme_name);

	// refresh UI with new values
	ui_set_current_theme_colors_to_styles();

	cleanup:
	if(theme_name){
		if(heap_caps_get_allocated_size(theme_name)) free(theme_name);
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* set colors for every style existing in UI
 * this function is used when user changes the theme */
static void ui_set_current_theme_colors_to_styles(void){

	const struct theme *current_theme;
	char *theme_name = 0;
	uint8_t a = 0;
	int res = 0, len = 0;

	// get current theme name from config
	NVS_GetConfig(CONFIG_THEME, &theme_name);
	if(0 == theme_name){

		ESP_LOGE("ui_styles.c", "theme_name = 0, trying to set %s as default theme", themes[0].theme_name);
		theme_name = themes[0].theme_name;
	}
	else{

		// get the length of theme name
		len = strnlen(theme_name, 32);
		if((0 == len) || (32 == len)) {

			ESP_LOGE("ui_styles.c", "string length error, trying to set %s as default theme", themes[0].theme_name);
			theme_name = themes[0].theme_name;
		}
	}

	// find the index of theme with obtained "theme name"
	while(NULL != themes[a].theme_name){

		res = memcmp(themes[a].theme_name, theme_name, len);
		if(0 == res) break;
		a++;
	}

	if(NULL == themes[a].theme_name){		// should never happen!!

		ESP_LOGE("ui_styles.c", "no theme found, trying to set %s as default theme", themes[0].theme_name);
		theme_name = themes[0].theme_name;
		a = 0;
	}

	// set pointer to the current theme
	current_theme = &themes[a];

	UI_CurrentTheme.background_color_base = lv_color_hex(current_theme->color_bckg_base_hex);
	UI_CurrentTheme.background_color_ext = lv_color_hex(current_theme->color_bckg_ext_hex);
	UI_CurrentTheme.main_color_base = lv_color_hex(current_theme->color_main_base_hex);
	UI_CurrentTheme.main_color_ext = lv_color_hex(current_theme->color_main_ext_hex);
	UI_CurrentTheme.contrast_color = lv_color_hex(current_theme->color_contr_hex);

	lv_style_set_bg_color(&UI_ScreenStyle, UI_CurrentTheme.background_color_base);
	lv_obj_report_style_change(&UI_ScreenStyle);

	lv_style_set_border_color(&UI_ButtonStyle, UI_CurrentTheme.main_color_base);
	lv_obj_report_style_change(&UI_ButtonStyle);

	lv_style_set_text_color(&UI_Icon24Style, UI_CurrentTheme.contrast_color);
	lv_obj_report_style_change(&UI_Icon24Style);

	lv_style_set_text_color(&UI_Icon16Style, UI_CurrentTheme.contrast_color);
	lv_obj_report_style_change(&UI_Icon16Style);

	lv_style_set_text_color(&UI_ClockStyle, UI_CurrentTheme.main_color_base);
	lv_obj_report_style_change(&UI_ClockStyle);

	lv_style_set_text_color(&UI_Text30Style, UI_CurrentTheme.main_color_ext);
	lv_obj_report_style_change(&UI_Text30Style);

	lv_style_set_text_color(&UI_Text14Style, UI_CurrentTheme.main_color_ext);
	lv_obj_report_style_change(&UI_Text14Style);

	lv_style_set_text_color(&UI_Text16Style, UI_CurrentTheme.main_color_ext);
	lv_obj_report_style_change(&UI_Text16Style);

	lv_style_set_text_color(&UI_Text16UnderlineStyle, UI_CurrentTheme.main_color_base);
	lv_obj_report_style_change(&UI_Text16UnderlineStyle);

	lv_style_set_arc_color(&UI_ArcRSSIStyle, UI_CurrentTheme.background_color_base);

	lv_style_set_border_color(&UI_CheckboxStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_text_color(&UI_CheckboxStyle, UI_CurrentTheme.contrast_color);
	lv_obj_report_style_change(&UI_CheckboxStyle);

	lv_style_set_bg_color(&UI_PanelStyle, UI_CurrentTheme.background_color_ext);
	lv_style_set_border_color(&UI_PanelStyle, UI_CurrentTheme.background_color_ext);
	lv_obj_report_style_change(&UI_PanelStyle);

	lv_style_set_bg_color(&UI_DropdownStyle, UI_CurrentTheme.background_color_ext);
	lv_style_set_border_color(&UI_DropdownStyle, UI_CurrentTheme.main_color_base);
	lv_style_set_text_color(&UI_DropdownStyle, UI_CurrentTheme.main_color_ext);
	lv_obj_report_style_change(&UI_DropdownStyle);
}
