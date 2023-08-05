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
	UI_EVT_BASIC_WEATHER_UPDATE,
	UI_EVT_DETAILED_WEATHER_UPDATE,

} UI_EventType_t;

typedef struct {

	char *icon_path;

	int temp;
	int press;
	int hum;

} UI_BasicWeatherValues_t;

typedef struct {

	char *city_name;
	char *country_name;

	char *icon_path;

	int average_temp;
	int min_temp;
	int max_temp;

	char *sunrise_time;
	char *sunset_time;

	int recip_rain;
	int percent_rain;

} UI_DetailedWeatherValues_t;

void UI_ReportEvt(UI_EventType_t Type, void *arg);
void UI_Task(void *arg);

#endif /* MAIN_INCLUDE_UI_TASK_H_ */
