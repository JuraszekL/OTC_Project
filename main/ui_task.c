#include "main.h"
#include "ui.h"

#define UI_EVT_RUN_STARTUP_SCREEN		0xFF
/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef void (*ui_event)(void *arg);

struct ui_queue_data {

	UI_EventType_t type;
	void *arg;
};

/**************************************************************
 *
 *	Function prototypes
 *
 **************************************************************/
static void ui_event_wifi_disconnected(void *arg);
static void ui_event_wifi_connected(void *arg);
static void ui_event_wifi_connecting(void *arg);
static void ui_event_wifi_connect_error(void *arg);
static void ui_event_wifi_get_pass(void *arg);
static void ui_event_wifilist_add(void *arg);
static void ui_event_wifilist_clear(void *arg);
static void ui_event_wifilist_clicked(void *arg);
static void ui_event_password_deleted(void *arg);
static void ui_event_password_not_deleted(void *arg);
static void ui_event_time_changed(void *arg);
static void ui_event_clock_not_sync(void *arg);
static void ui_event_clock_sync(void *arg);
static void ui_event_basic_weather_update(void *arg);
static void ui_event_detailed_weather_update(void *arg);
static void ui_event_main_scr_wifi_btn_clicked(void *arg);
static void ui_event_wifi_scr_back_btn_clicked(void *arg);
static void ui_event_weather_scr_back_btn_clicked(void *arg);
static void ui_event_startup_screen_ready(void *arg);
static void ui_event_setup_scr_back_btn_clicked(void *arg);
static void ui_event_main_scr_setup_btn_clicked(void *arg);
static void ui_event_wifi_popup_delete_request(void *arg);
static void ui_event_theme_change_request(void *arg);

static void ui_event_run_startup_screen(void *arg);
/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/

const ui_event event_tab[] = {

		[UI_EVT_WIFI_CONNECTED] = ui_event_wifi_connected,
		[UI_EVT_WIFI_DISCONNECTED] = ui_event_wifi_disconnected,
		[UI_EVT_WIFI_CONNECTING] = ui_event_wifi_connecting,
		[UI_EVT_WIFI_CONNECT_ERROR] = ui_event_wifi_connect_error,
		[UI_EVT_WIFI_GET_PASS] = ui_event_wifi_get_pass,
		[UI_EVT_WIFI_LIST_ADD] = ui_event_wifilist_add,
		[UI_EVT_WIFI_LIST_CLEAR] = ui_event_wifilist_clear,
		[UI_EVT_WIFI_LIST_CLICKED] = ui_event_wifilist_clicked,
		[UI_EVT_WIFI_PASS_DELETED] = ui_event_password_deleted,
		[UI_EVT_WIFI_PASS_NOT_DELETED] = ui_event_password_not_deleted,
		[UI_EVT_TIME_CHANGED] = ui_event_time_changed,
		[UI_EVT_CLOCK_NOT_SYNC] = ui_event_clock_not_sync,
		[UI_EVT_CLOCK_SYNC] = ui_event_clock_sync,
		[UI_EVT_BASIC_WEATHER_UPDATE] = ui_event_basic_weather_update,
		[UI_EVT_DETAILED_WEATHER_UPDATE] = ui_event_detailed_weather_update,
		[UI_EVT_MAINSCR_WIFI_BTN_CLICKED] = ui_event_main_scr_wifi_btn_clicked,
		[UI_EVT_WIFISCR_BACK_BTN_CLICKED] = ui_event_wifi_scr_back_btn_clicked,
		[UI_EVT_WEATHERSCR_BACK_BTN_CLICKED] =  ui_event_weather_scr_back_btn_clicked,
		[UI_EVT_STARTUP_SCREEN_READY] = ui_event_startup_screen_ready,
		[UI_EVT_SETUPSCR_BACK_BTN_CLICKED] = ui_event_setup_scr_back_btn_clicked,
		[UI_EVT_MAINSCR_SETUP_BTN_CLICKED] = ui_event_main_scr_setup_btn_clicked,
		[UI_EVT_WIFI_POPUP_DELETE_REQUEST] = ui_event_wifi_popup_delete_request,
		[UI_EVT_THEME_CHANGE_REQUEST] = ui_event_theme_change_request,

		[UI_EVT_RUN_STARTUP_SCREEN] = ui_event_run_startup_screen,
};

static QueueHandle_t ui_queue_handle;


/******************************************************************************************************************
 *
 * UI task
 *
 ******************************************************************************************************************/
void UI_Task(void *arg){

	BaseType_t ret;
	struct ui_queue_data data;

	// create UI events queue
	ui_queue_handle = xQueueCreate(5U, sizeof(struct ui_queue_data));
	assert(ui_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, UI_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
	UI_InitStyles();

	UI_StartupScreen_Init();
	UI_MainScreen_Init();
	UI_WeatherScreen_Init();
	UI_WifiScreen_Init();
	UI_SetupScreen_Init();

	xSemaphoreGive(LVGL_MutexHandle);

	UI_ReportEvt(UI_EVT_RUN_STARTUP_SCREEN, 0);

	while(1){

		ret = xQueueReceive(ui_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			ret = xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
			if(pdTRUE == ret){

				event_tab[data.type](data.arg);
		    	xSemaphoreGive(LVGL_MutexHandle);
			}
		}
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

void UI_ReportEvt(UI_EventType_t Type, void *arg){

	struct ui_queue_data data;
	BaseType_t res;

	data.type = Type;
	data.arg = arg;

	res = xQueueSend(ui_queue_handle, &data, pdMS_TO_TICKS(100));
	if(pdPASS != res){

		if(arg){
			if(heap_caps_get_allocated_size(arg)) free(arg);
		}
	}
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/*****************************
 * UI event functions
 * ***************************/

static void ui_event_wifi_disconnected(void *arg){

	// set wifi icon in top left corner of main screen
	UI_MainScreen_SetTopIcon(wifi_icon, ICON_NO_WIFI);

	// clear information about connected AP
	UI_WifiScreen_SetApDetails(0);
}

static void ui_event_wifi_connected(void *arg){

	UI_DetailedAPData_t *data = (UI_DetailedAPData_t *)arg;

	// set wifi icon in top left corner of main screen
	UI_MainScreen_SetTopIcon(wifi_icon, ICON_WIFI);

	// create popup with information that wifi is connected
	UI_WifiScreen_PopupConnected(data);

	// set information about connected AP on wifi screen
	UI_WifiScreen_SetApDetails(data);

	// free resources
	if(data){

		if(data->ssid){

			if(heap_caps_get_allocated_size(data->ssid)) free(data->ssid);
		}
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_wifi_connecting(void *arg){

	if(0 == arg) return;

	WifiCreds_t *creds = (WifiCreds_t *)arg;

	//TODO set animation to main screen wifi symbol

	// create popup with information that wifi is trying to conenct
	UI_WifiScreen_PopupConnecting(creds);

	// free resources
	if(creds){

		if(creds->ssid){
			if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
		}
		if(creds->pass){
			if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
		}
		if(heap_caps_get_allocated_size(creds)) free(creds);
	}
}

static void ui_event_wifi_connect_error(void *arg){

	UI_WifiScreen_PopupConnectError();

	//TODO delete animation of wifi icon
}

static void ui_event_wifi_get_pass(void *arg){

	if(0 == arg) return;

	WifiCreds_t *creds = (WifiCreds_t *)arg;

	UI_WifiScreen_PopupGetPass(creds);
}

// add single element to list of found wifi networks
static void ui_event_wifilist_add(void *arg){

	if(0 == arg) return;

	UI_BasicAPData_t *data = (UI_BasicAPData_t *)arg;

	UI_WifiScreen_WifiListAdd(data);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_wifilist_clear(void *arg){

	UI_WifiScreen_WifiListClear();
}

static void ui_event_time_changed(void *arg){

	if(0 == arg) return;

	struct tm *changed_time = (struct tm *)arg;

	UI_MainScreen_UpdateTimeAndDate(changed_time);
}

static void ui_event_clock_not_sync(void *arg){

	UI_MainScreen_SetTopIcon(sync_icon, ICON_NO_SYNC);
}


static void ui_event_clock_sync(void *arg){

	UI_MainScreen_SetTopIcon(sync_icon, ICON_SYNC);

}


static void ui_event_basic_weather_update(void *arg){

	if(0 == arg) return;

	UI_BasicWeatherValues_t *data = (UI_BasicWeatherValues_t *)arg;

	UI_MainScreen_UpdateWeatherIcon(data->icon_path);
	UI_MainScreen_UpdateWeatherData(data->temp, data->press, data->hum);

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_detailed_weather_update(void *arg){

	if(0 == arg) return;

	UI_DetailedWeatherValues_t *data = (UI_DetailedWeatherValues_t *)arg;

	UI_WeatherScreen_UpdateCityName(data->city_name);
	UI_WeatherScreen_UpdateCountryName(data->country_name);
	UI_WeatherScreen_UpdateWeatherIcon(data->icon_path);
	UI_WeatherScreen_UpdateAvgTemp(data->average_temp);
	UI_WeatherScreen_UpdateMinMaxTemp(data->min_temp, data->max_temp);
	UI_WeatherScreen_UpdateSunriseSunsetTime(data->sunrise_time, data->sunset_time);
	UI_WeatherScreen_UpdatePrecipPercentRain(data->recip_rain, data->percent_rain);
	UI_WeatherScreen_UpdateAvgMaxWind(data->wind_avg, data->wind_max);
	UI_WeatherScreen_UpdatePrecipPercentSnow(data->recip_snow, data->percent_snow);

	UI_WeatherScreen_Load();

	if(data){
		if(heap_caps_get_allocated_size(data)) free(data);
	}
}

static void ui_event_main_scr_wifi_btn_clicked(void *arg){

	UI_WifiScreen_Load(0);
	WIFI_StartScan();
}

static void ui_event_wifi_scr_back_btn_clicked(void *arg){

	UI_MainScreen_Load(0);
}

static void ui_event_weather_scr_back_btn_clicked(void *arg){

	UI_MainScreen_Load(0);
}

static void ui_event_startup_screen_ready(void *arg){

	UI_StartupScreen_Cleanup();
	UI_MainScreen_Load(2000);
}

static void ui_event_wifilist_clicked(void *arg){

	if(0 == arg) return;

	char *obj = (char *)arg;

	UI_WifiScreen_WifiListClicked(obj);
}

static void ui_event_password_deleted(void *arg){

	if(0 == arg) return;

	char *obj = (char *)arg;

	UI_WifiScreen_PopupPassDeleted(obj);
}

static void ui_event_password_not_deleted(void *arg){

	if(0 == arg) return;

	char *obj = (char *)arg;

	UI_WifiScreen_PopupPassNotDeleted(obj);
}

static void ui_event_run_startup_screen(void *arg){

	UI_StarttupScreen_Load();
}

static void ui_event_setup_scr_back_btn_clicked(void *arg){

	UI_MainScreen_Load(0);
}

static void ui_event_main_scr_setup_btn_clicked(void *arg){

	UI_SetupScreen_Load();
}

static void ui_event_wifi_popup_delete_request(void *arg){

	UI_WifiScreen_PopupDelete();
}

static void ui_event_theme_change_request(void *arg){

	char *theme_name = (char *)arg;

	UI_ChangeTheme(theme_name);
}
