#ifndef MAIN_INCLUDE_UI_TASK_H_
#define MAIN_INCLUDE_UI_TASK_H_

/**************************************************************
 * UI event types
 ***************************************************************/
typedef enum {

	UI_EVT_WIFI_CONNECTED = 0,
	UI_EVT_WIFI_DISCONNECTED,
	UI_EVT_WIFI_CONNECTING,
	UI_EVT_WIFI_CONNECT_ERROR,
	UI_EVT_WIFI_GET_PASS,
	UI_EVT_WIFI_LIST_ADD,
	UI_EVT_WIFI_LIST_CLEAR,
	UI_EVT_WIFI_LIST_CLICKED,
	UI_EVT_TIME_CHANGED,
	UI_EVT_CLOCK_NOT_SYNC,
	UI_EVT_CLOCK_SYNC,
	UI_EVT_BASIC_WEATHER_UPDATE,
	UI_EVT_DETAILED_WEATHER_UPDATE,
	UI_EVT_MAINSCR_WIFI_BTN_CLICKED,
	UI_EVT_WIFISCR_BACK_BTN_CLICKED,
	UI_EVT_WEATHERSCR_BACK_BTN_CLICKED,
	UI_EVT_STARTUP_SCREEN_READY,
	UI_EVT_SETUPSCR_BACK_BTN_CLICKED,
	UI_EVT_MAINSCR_SETUP_BTN_CLICKED,

} UI_EventType_t;

/**************************************************************
 * Data with basic weather values
 ***************************************************************/
typedef struct {

	char *icon_path;

	int temp;
	int press;
	int hum;

} UI_BasicWeatherValues_t;

/**************************************************************
 * Data with detailed weather values
 ***************************************************************/
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

	int wind_avg;
	int wind_max;

	int recip_snow;
	int percent_snow;

} UI_DetailedWeatherValues_t;


/**************************************************************
 * Public functions
 ***************************************************************/
void UI_ReportEvt(UI_EventType_t Type, void *arg);
void UI_Task(void *arg);

#endif /* MAIN_INCLUDE_UI_TASK_H_ */
