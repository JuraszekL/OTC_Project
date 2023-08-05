#ifndef MAIN_INCLUDE_UI_TASK_H_
#define MAIN_INCLUDE_UI_TASK_H_

#define ICON_WIFI			'A'
#define ICON_NO_WIFI		'B'
#define ICON_SYNC			'C'
#define ICON_NO_SYNC		'D'
#define ICON_UP_ARROW		'E'
#define ICON_LEFT_ARROW		'F'
#define ICON_RIGHT_ARROW	'G'
#define ICON_DOWN_ARROW		'H'

typedef enum {

	UI_EVT_WIFI_CONNECTED = 0,
	UI_EVT_WIFI_DISCONNECTED,
	UI_EVT_TIME_CHANGED,
	UI_EVT_CLOCK_NOT_SYNC,
	UI_EVT_CLOCK_SYNC,
	UI_EVT_BASIC_WEATHER_ICON_UPDATE,
	UI_EVT_BASIC_WEATHER_VALUES_UPDATE,
	UI_EVT_WEATHER_CITY_UPDATE,
	UI_EVT_WEATHER_COUNTRY_UPDATE,
	UI_EVT_WEATHER_ICON_UPDATE,
	UI_EVT_WEATHER_AVG_TEMP_UPDATE,

} UI_EventType_t;

typedef struct {

	int temp;
	int press;
	int hum;

} UI_WeatherValues_t;

void UI_ReportEvt(UI_EventType_t Type, void *arg);

void UI_Task(void *arg);

#endif /* MAIN_INCLUDE_UI_TASK_H_ */
