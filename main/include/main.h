#ifndef MAIN_INCLUDE_MAIN_H_
#define MAIN_INCLUDE_MAIN_H_

#define LV_KCONFIG_IGNORE

#define VERSION_MAJOR		"0"
#define VERSION_MINOR		"0"
#define VERSION_PATCH		"4"

#define MAIN_TASK_BIT			(1 << 0)
#define DISPLAY_TASK_BIT		(1 << 1)
#define TOUCHPAD_TASK_BIT		(1 << 2)
#define UI_TASK_BIT				(1 << 3)
#define WIFI_TASK_BIT			(1 << 4)
#define CLOCK_TASK_BIT			(1 << 5)
#define ONLINEREQS_TASK_BIT		(1 << 6)
#define SDCARD_TASK_BIT			(1 << 7)

#define ALL_TASKS_BITS		(MAIN_TASK_BIT | DISPLAY_TASK_BIT | TOUCHPAD_TASK_BIT | UI_TASK_BIT \
							| WIFI_TASK_BIT |CLOCK_TASK_BIT | ONLINEREQS_TASK_BIT | SDCARD_TASK_BIT)

#define TIMEOUT_MS				5000

extern EventGroupHandle_t AppStartSyncEvt;
extern TimerHandle_t OneSecondTimer;

void * lvgl_malloc(size_t size);
void lvgl_free(void * data);
void * lvgl_realloc(void * data_p, size_t new_size);

#endif /* MAIN_INCLUDE_MAIN_H_ */
