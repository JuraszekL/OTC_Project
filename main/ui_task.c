/**************************************************************
 *	include
 ***************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lvgl.h"
#include "ui.h"

#include "main.h"
#include "animations.h"
#include "ui_task.h"

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
void startup_screen(void);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
const char version[] = {VERSION};

/******************************************************************************************************************
 *
 * UI task
 *
 ******************************************************************************************************************/
void UI_Task(void *arg){

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, UI_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// run startup screen
	startup_screen();

	ui_MainScreen_screen_init();
	vTaskDelay(1);

	lv_label_set_text(ui_WeatherIcon, "\uF001");
	lv_scr_load_anim(ui_MainScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 150, 2000, true);

	while(1){

		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* setup and display startup screend */
void startup_screen(void){

	// init startup screen
	ui_StartupScreen_screen_init();
	vTaskDelay(1);

	// hide all elements
	lv_obj_set_style_bg_opa(ui_StartupScreenPanel, 0, 0);
	lv_obj_set_style_text_opa(ui_OnlineTableClockLabel, 0, 0);
	lv_obj_set_style_text_opa(ui_ByJuraszekLLabel, 0, 0);
	lv_obj_set_style_shadow_opa(ui_StartupScreenPanel, 0, 0);
	lv_obj_add_flag(ui_LogosImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_VersionLabel, LV_OBJ_FLAG_HIDDEN);

	// set version number to label
	lv_label_set_text(ui_VersionLabel, version);

	// load screen
	lv_disp_load_scr(ui_StartupScreen);

	// start animation
	Anm_InitScr2200msOpa();

	// show image with logos and version label
	vTaskDelay(pdMS_TO_TICKS(2500));
	lv_obj_clear_flag(ui_LogosImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(ui_VersionLabel, LV_OBJ_FLAG_HIDDEN);
}
