#include <stdio.h>
#include "lvgl.h"

#include "ui.h"

#include "animations.h"

static void set_bg_opa(void *obj, int32_t opa);
static void set_shad_opa(void *obj, int32_t opa);
static void set_text_opa(void *obj, int32_t opa);

void Anm_InitScr1200msOpa(void){

	lv_anim_t a, b, c, d;
	lv_anim_init(&a);
	lv_anim_init(&b);
	lv_anim_init(&c);
	lv_anim_init(&d);

	/*Set the "animator" function*/
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) set_bg_opa);
	lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t) set_shad_opa);
	lv_anim_set_exec_cb(&c, (lv_anim_exec_xcb_t) set_text_opa);
	lv_anim_set_exec_cb(&d, (lv_anim_exec_xcb_t) set_text_opa);

	/*Set target of the animation*/
	lv_anim_set_var(&a, ui_InitScreenPanel);
	lv_anim_set_var(&b, ui_InitScreenPanel);
	lv_anim_set_var(&c, ui_OnlineTableClockLabel);
	lv_anim_set_var(&d, ui_ByJuraszekLLabel);

	/*Length of the animation [ms]*/
	lv_anim_set_time(&a, 1200);
	lv_anim_set_time(&b, 1200);
	lv_anim_set_time(&c, 100);
	lv_anim_set_time(&d, 100);

	/*Set start and end values. E.g. 0, 150*/
	lv_anim_set_values(&a, 0, 255);
	lv_anim_set_values(&b, 0, 255);
	lv_anim_set_values(&c, 0, 255);
	lv_anim_set_values(&d, 0, 255);

	lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
	lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
	lv_anim_set_path_cb(&c, lv_anim_path_ease_in);
	lv_anim_set_path_cb(&d, lv_anim_path_ease_in);

	lv_anim_set_delay(&c, 2000);
	lv_anim_set_delay(&d, 2100);

	lv_anim_start(&a);
	lv_anim_start(&b);
	lv_anim_start(&c);
	lv_anim_start(&d);
}


static void set_bg_opa(void *obj, int32_t opa){

	lv_obj_set_style_bg_opa((lv_obj_t *)obj, opa, 0);
}

static void set_shad_opa(void *obj, int32_t opa){

	lv_obj_set_style_shadow_opa((lv_obj_t *)obj, opa, 0);
}

static void set_text_opa(void *obj, int32_t opa){

	lv_obj_set_style_text_opa((lv_obj_t *)obj, opa, 0);
}
