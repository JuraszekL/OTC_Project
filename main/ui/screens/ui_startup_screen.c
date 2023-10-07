#include "ui.h"

/**************************************************************
 *
 *	Macros
 *
 ***************************************************************/
#define VERSION	("v." VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH)

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_startup_screen_unloaded(lv_event_t * e);

static void ui_startup_screen_animations_init(void);
static void author_animation_ready(struct _lv_anim_t *a);
static void set_bg_opa(void *obj, int32_t opa);
static void set_shad_opa(void *obj, int32_t opa);
static void set_text_opa(void *obj, int32_t opa);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *ui_StartupScreen, *ui_StartupScreenPanel, *ui_StartupScreenPanelLabel, *ui_StartupScreenTitleLabel,
				*ui_StartupScreenAuthorLabel, *ui_StartupScreenLogosImage, *ui_StartupScreenVersionLabel;

static lv_anim_timeline_t *ui_startup_timeline;

static const char version[] = {VERSION};

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize startup screen */
void UI_StartupScreen_Init(void){

	UI_ScreenCreate(&ui_StartupScreen);
	lv_obj_add_event_cb(ui_StartupScreen, ui_startup_screen_unloaded, LV_EVENT_SCREEN_UNLOADED, 0);

	ui_StartupScreenPanel = lv_obj_create(ui_StartupScreen);
	lv_obj_set_size(ui_StartupScreenPanel, 280, 100);
	lv_obj_set_x(ui_StartupScreenPanel, 0);
	lv_obj_set_y(ui_StartupScreenPanel, 50);
	lv_obj_set_align(ui_StartupScreenPanel, LV_ALIGN_TOP_MID);
	lv_obj_clear_flag(ui_StartupScreenPanel, LV_OBJ_FLAG_SCROLLABLE );     /// Flags
	lv_obj_set_style_bg_opa(ui_StartupScreenPanel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_StartupScreenPanel, UI_CurrentTheme.main_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui_StartupScreenPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui_StartupScreenPanel, UI_CurrentTheme.main_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui_StartupScreenPanel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui_StartupScreenPanel, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui_StartupScreenPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_StartupScreenPanelLabel = lv_label_create(ui_StartupScreenPanel);
	lv_obj_set_width(ui_StartupScreenPanelLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_StartupScreenPanelLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_align(ui_StartupScreenPanelLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_StartupScreenPanelLabel, "OTC PROJECT");
	lv_obj_set_style_bg_opa(ui_StartupScreenPanelLabel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_StartupScreenPanelLabel, &ui_font_Varino30, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_StartupScreenPanelLabel, UI_CurrentTheme.background_color_base,
			LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_StartupScreenPanelLabel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_StartupScreenTitleLabel = lv_label_create(ui_StartupScreen);
	lv_obj_set_width(ui_StartupScreenTitleLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_StartupScreenTitleLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_StartupScreenTitleLabel, 0);
	lv_obj_set_y(ui_StartupScreenTitleLabel, 170);
	lv_obj_set_align(ui_StartupScreenTitleLabel, LV_ALIGN_TOP_MID);
	lv_label_set_text(ui_StartupScreenTitleLabel, "ONLINE TABLE CLOCK");
	lv_obj_set_style_bg_opa(ui_StartupScreenTitleLabel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_StartupScreenTitleLabel, &ui_font_Varino18, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_StartupScreenTitleLabel, UI_CurrentTheme.main_color_ext,
			LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_StartupScreenTitleLabel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_StartupScreenAuthorLabel = lv_label_create(ui_StartupScreen);
	lv_obj_set_width(ui_StartupScreenAuthorLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_StartupScreenAuthorLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_StartupScreenAuthorLabel, 0);
	lv_obj_set_y(ui_StartupScreenAuthorLabel, -35);
	lv_obj_set_align(ui_StartupScreenAuthorLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_StartupScreenAuthorLabel, "by JuraszekL");
	lv_obj_set_style_bg_opa(ui_StartupScreenAuthorLabel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_StartupScreenAuthorLabel, &ui_font_Varino12, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_StartupScreenAuthorLabel, UI_CurrentTheme.contrast_color,
			LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_StartupScreenAuthorLabel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_StartupScreenLogosImage = lv_img_create(ui_StartupScreen);
	lv_img_set_src(ui_StartupScreenLogosImage, &ui_img_logos_png);
	lv_obj_set_width(ui_StartupScreenLogosImage, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_StartupScreenLogosImage, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_StartupScreenLogosImage, 0);
	lv_obj_set_y(ui_StartupScreenLogosImage, -10);
	lv_obj_set_align(ui_StartupScreenLogosImage, LV_ALIGN_BOTTOM_MID);
	lv_obj_add_flag(ui_StartupScreenLogosImage, LV_OBJ_FLAG_HIDDEN);     /// Flags
	lv_obj_clear_flag(ui_StartupScreenLogosImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

	ui_StartupScreenVersionLabel = lv_label_create(ui_StartupScreen);
	lv_obj_add_style(ui_StartupScreenVersionLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_width(ui_StartupScreenVersionLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_StartupScreenVersionLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_StartupScreenVersionLabel, -5);
	lv_obj_set_y(ui_StartupScreenVersionLabel, 5);
	lv_obj_set_align(ui_StartupScreenVersionLabel, LV_ALIGN_TOP_RIGHT);
	lv_label_set_text(ui_StartupScreenVersionLabel, version);
	lv_obj_add_flag(ui_StartupScreenVersionLabel, LV_OBJ_FLAG_HIDDEN);

	ui_startup_screen_animations_init();
}

/* load startup screen */
void UI_StarttupScreen_Load(void){

	lv_disp_load_scr(ui_StartupScreen);
	lv_anim_timeline_start(ui_startup_timeline);
}

/* cleanup after animations are done */
void UI_StartupScreen_Cleanup(void){

	lv_anim_del(ui_StartupScreenPanel, NULL);
	lv_anim_del(ui_StartupScreenTitleLabel, NULL);
	lv_anim_del(ui_StartupScreenAuthorLabel, NULL);

	lv_obj_clear_flag(ui_StartupScreenLogosImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(ui_StartupScreenVersionLabel, LV_OBJ_FLAG_HIDDEN);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* delete startup screen if has been unloaded */
static void ui_startup_screen_unloaded(lv_event_t * e){

	lv_obj_del_async(ui_StartupScreen);
}

/* initalize animations for startup screen */
static void ui_startup_screen_animations_init(void){

	lv_anim_t startup_panel_animation, startup_panel_shadow_animation, startup_title_animation,
				startup_author_animation;

	lv_anim_init(&startup_panel_animation);
	lv_anim_set_exec_cb(&startup_panel_animation, (lv_anim_exec_xcb_t) set_bg_opa);
	lv_anim_set_var(&startup_panel_animation, ui_StartupScreenPanel);
	lv_anim_set_time(&startup_panel_animation, 1200);
	lv_anim_set_values(&startup_panel_animation, 0, 255);
	lv_anim_set_path_cb(&startup_panel_animation, lv_anim_path_ease_in);

	lv_anim_init(&startup_panel_shadow_animation);
	lv_anim_set_exec_cb(&startup_panel_shadow_animation, (lv_anim_exec_xcb_t) set_shad_opa);
	lv_anim_set_var(&startup_panel_shadow_animation, ui_StartupScreenPanel);
	lv_anim_set_time(&startup_panel_shadow_animation, 1200);
	lv_anim_set_values(&startup_panel_shadow_animation, 0, 255);
	lv_anim_set_path_cb(&startup_panel_shadow_animation, lv_anim_path_ease_in);

	lv_anim_init(&startup_title_animation);
	lv_anim_set_exec_cb(&startup_title_animation, (lv_anim_exec_xcb_t) set_text_opa);
	lv_anim_set_var(&startup_title_animation, ui_StartupScreenTitleLabel);
	lv_anim_set_time(&startup_title_animation, 250);
	lv_anim_set_values(&startup_title_animation, 0, 255);
	lv_anim_set_path_cb(&startup_title_animation, lv_anim_path_ease_in);

	lv_anim_init(&startup_author_animation);
	lv_anim_set_exec_cb(&startup_author_animation, (lv_anim_exec_xcb_t) set_text_opa);
	lv_anim_set_var(&startup_author_animation, ui_StartupScreenAuthorLabel);
	lv_anim_set_time(&startup_author_animation, 250);
	lv_anim_set_values(&startup_author_animation, 0, 255);
	lv_anim_set_path_cb(&startup_author_animation, lv_anim_path_ease_in);
	lv_anim_set_ready_cb(&startup_author_animation, author_animation_ready);

	ui_startup_timeline = lv_anim_timeline_create();
	lv_anim_timeline_add(ui_startup_timeline, 0, &startup_panel_animation);
	lv_anim_timeline_add(ui_startup_timeline, 0, &startup_panel_shadow_animation);
	lv_anim_timeline_add(ui_startup_timeline, 2000, &startup_title_animation);
	lv_anim_timeline_add(ui_startup_timeline, 2250, &startup_author_animation);
}

/* callback when animations are done */
static void author_animation_ready(struct _lv_anim_t *a){

	UI_ReportEvt(UI_EVT_STARTUP_SCREEN_READY, 0);
}

/**************************************************************
 * Animation helpers
 ***************************************************************/

// change background opacity of default style
static void set_bg_opa(void *obj, int32_t opa){

	lv_obj_set_style_bg_opa((lv_obj_t *)obj, opa, 0);
}

// change shadow opacity of default style
static void set_shad_opa(void *obj, int32_t opa){

	lv_obj_set_style_shadow_opa((lv_obj_t *)obj, opa, 0);
}

// change text opacity of default style
static void set_text_opa(void *obj, int32_t opa){

	lv_obj_set_style_text_opa((lv_obj_t *)obj, opa, 0);
}
