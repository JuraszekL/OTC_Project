#ifndef MAIN_INCLUDE_MAIN_H_
#define MAIN_INCLUDE_MAIN_H_

#define VERSION_MAJOR		"0"
#define VERSION_MINOR		"0"
#define VERSION_PATCH		"2"

#define MAIN_TASK_BIT		(1 << 0)
#define DISPLAY_TASK_BIT	(1 << 1)
#define TOUCHPAD_TASK_BIT	(1 << 2)
#define UI_TASK_BIT			(1 << 3)
#define WIFI_TASK_BIT		(1 << 4)
#define SNTP_TASK_BIT		(1 << 5)

#define ALL_TASKS_BITS		(MAIN_TASK_BIT | DISPLAY_TASK_BIT | TOUCHPAD_TASK_BIT | UI_TASK_BIT \
							| WIFI_TASK_BIT |SNTP_TASK_BIT)

extern EventGroupHandle_t AppStartSyncEvt;

#endif /* MAIN_INCLUDE_MAIN_H_ */
