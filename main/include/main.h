#ifndef MAIN_INCLUDE_MAIN_H_
#define MAIN_INCLUDE_MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/**************************************************************
 * Use lv_conf.h instead of kconfig
 ***************************************************************/
#define LV_KCONFIG_IGNORE

/**************************************************************
 * Project version
 ***************************************************************/
#define VERSION_MAJOR		"0"
#define VERSION_MINOR		"0"
#define VERSION_PATCH		"6"

/**************************************************************
 * Bits for aplication startup synchro
 ***************************************************************/
#define MAIN_TASK_BIT			(1 << 0)
#define DISPLAY_TASK_BIT		(1 << 1)
#define TOUCHPAD_TASK_BIT		(1 << 2)
#define UI_TASK_BIT				(1 << 3)
#define WIFI_TASK_BIT			(1 << 4)
#define CLOCK_TASK_BIT			(1 << 5)
#define ONLINEREQS_TASK_BIT		(1 << 6)
#define SDCARD_TASK_BIT			(1 << 7)
#define SPIFFS_TASK_BIT			(1 << 8)

#define ALL_TASKS_BITS		(MAIN_TASK_BIT | DISPLAY_TASK_BIT | TOUCHPAD_TASK_BIT | UI_TASK_BIT \
							| WIFI_TASK_BIT |CLOCK_TASK_BIT | ONLINEREQS_TASK_BIT | SDCARD_TASK_BIT \
							| SPIFFS_TASK_BIT)

/**************************************************************
 * General timeout in online operations
 ***************************************************************/
#define TIMEOUT_MS				5000

/**************************************************************
 * Public structure with Wifi credentials
 ***************************************************************/
typedef struct {

	char *ssid;
	char *pass;
	bool save;

} WifiCreds_t;

/**************************************************************
 * Public variables
 ***************************************************************/
extern EventGroupHandle_t AppStartSyncEvt;
extern TimerHandle_t OneSecondTimer;

/**************************************************************
 * Public functions
 ***************************************************************/
void * lvgl_malloc(size_t size);
void lvgl_free(void * data);
void * lvgl_realloc(void * data_p, size_t new_size);

#endif /* MAIN_INCLUDE_MAIN_H_ */
