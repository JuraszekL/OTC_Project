#ifndef MAIN_INCLUDE_UI_TASK_H_
#define MAIN_INCLUDE_UI_TASK_H_

#define ICON_WIFI		"A"
#define ICON_NO_WIFI	"B"
#define ICON_WIFI_ERR	"C"

typedef enum {

	UI_EVT_WIFI_CONNECTED = 0,
	UI_EVT_WIFI_DISCONNECTED,

} UI_EventType_t;

void UI_ReportEvt(UI_EventType_t Type, void *arg);

void UI_Task(void *arg);

#endif /* MAIN_INCLUDE_UI_TASK_H_ */
