#include "main.h"
#include "driver/gptimer.h"
#include "esp_wifi.h"
#include <string.h>

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/

/* status bits */
#define WIFI_CONNECTED_BIT					(1U << 0)
#define WIFI_DISCONNECTED_BIT				(1U << 1)
#define WIFI_AUTOCONNECT_ENABLE_BIT			(1U << 2)
#define WIFI_SCAN_IN_PROGRESS_BIT			(1U << 3)
#define WIFI_CONNECTION_IN_PROGRESS_BIT		(1U << 4)
#define WIFI_SAVE_PASSWORD_BIT				(1U << 5)

/* other definitions */
#define WIFI_RECONNECT_ATTEMPTS				3

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef enum {

	wifi_start = 0,
	wifi_scan_start,
	wifi_scan_done,
	wifi_disconnected,
	wifi_connected,
	wifi_sta_got_ip,
	wifi_connect,

} wifi_routine_t;

typedef void (*wifi_routine)(void *arg);

struct wifi_queue_data {

	wifi_routine_t type;
	void *arg;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void network_init(void);
static void wifi_routine_request(wifi_routine_t type, void *arg);
static void wifi_evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

static void wifi_start_routine(void *arg);
static void wifi_scan_start_routine(void *arg);
static void wifi_scan_done_routine(void *arg);
static void wifi_disconnected_routine(void *arg);
static void wifi_connected_routine(void *arg);
static void wifi_sta_got_ip_routine(void *arg);
static void wifi_connect_routine(void *arg);

static void fill_wifi_list_with_found_aps(void);

/**************************************************************
 *
 *	Global variables
 *
 **************************************************************/
static EventGroupHandle_t WifiEvents;
static QueueHandle_t wifi_queue_handle;
static SemaphoreHandle_t wifi_reconnect_semaphore_handle;
static const wifi_routine wifi_routines_tab[] = {

		[wifi_start] = wifi_start_routine,
		[wifi_scan_start] = wifi_scan_start_routine,
		[wifi_scan_done] = wifi_scan_done_routine,
		[wifi_disconnected] = wifi_disconnected_routine,
		[wifi_connected] = wifi_connected_routine,
		[wifi_sta_got_ip] = wifi_sta_got_ip_routine,
		[wifi_connect] = wifi_connect_routine,
};

static wifi_ap_record_t *ap_list;
static uint16_t ap_count;

static esp_netif_t *station_netif_obj;

/******************************************************************************************************************
 *
 * Wifi task
 *
 ******************************************************************************************************************/
void Wifi_Task(void *arg){

	BaseType_t ret;
	struct wifi_queue_data data;

	// create wifi events group
	WifiEvents = xEventGroupCreate();
	assert(WifiEvents);

	// create wifi routines queue
	wifi_queue_handle = xQueueCreate(3U, sizeof(struct wifi_queue_data));
	assert(wifi_queue_handle);

	// initialize WiFi and NETIF
	network_init();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, WIFI_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// recieve routine
		ret = xQueueReceive(wifi_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// perform routine
			wifi_routines_tab[data.type](data.arg);
		}
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Block until the wifi is connected or Time_ms expires */
bool Wifi_WaitUntilIsConnected(unsigned int Time_ms){

	EventBits_t bits =  xEventGroupWaitBits(WifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE,
			pdMS_TO_TICKS(Time_ms));
	if(bits & WIFI_CONNECTED_BIT) return true;
	else return false;
}

/* run scan manually */
void WIFI_StartScan(void){

	wifi_routine_request(wifi_scan_start, NULL);
}

/* function called from UI when user requests to connect with "clicked" wifi  or
 * when user has put the password for requested wifi */
void Wifi_Connect(WifiCreds_t *creds){

	if((0 == creds) || (0 == creds->ssid)) return;

	EventBits_t bits = xEventGroupGetBits(WifiEvents);

	// return if already trying to connect
	if(bits & WIFI_CONNECTION_IN_PROGRESS_BIT) return;;

	if(0 == creds->pass){

		ESP_LOGI("wifi.c", "no password in creds, get pass from SPIFFS");
		SPIFFS_GetPass(creds);
	}
	else{

		ESP_LOGI("wifi.c", "password: %s, connecting", creds->pass);
		wifi_routine_request(wifi_connect, creds);
	}
}

/* function called by filesystem when searching for password has been finished
 * if no password found, creds->pass == 0, and function runs UI to obtain password from user */
void Wifi_ReportPass(WifiCreds_t *creds){

	if((0 == creds) || (0 == creds->ssid)) return;

	if(0 == creds->pass){

		UI_ReportEvt(UI_EVT_WIFI_GET_PASS, creds);
	}
	else{

		wifi_routine_request(wifi_connect, creds);
	}
}
/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* initialize wifi peripherials and NETIF layer */
static void network_init(void){

	wifi_init_config_t sta_init_config = WIFI_INIT_CONFIG_DEFAULT(); // @suppress("Symbol is not resolved")

	// initialize network interface in tcp/ip stack
	ESP_ERROR_CHECK(esp_netif_init());

	// create netif object with typical wifi station configuration
	station_netif_obj = esp_netif_create_default_wifi_sta();
	assert(station_netif_obj);

	// initialize wifi
	ESP_ERROR_CHECK(esp_wifi_init(&sta_init_config));

	// set device as station
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	// install wifi_evt_handler function as a handler for needed wifi events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_evt_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_evt_handler, NULL, NULL));

	// start wifi
	ESP_ERROR_CHECK(esp_wifi_start());
}

/* send request of routine to be performed */
static void wifi_routine_request(wifi_routine_t type, void *arg){

	struct wifi_queue_data data;
	BaseType_t res;

	data.type = type;
	data.arg = arg;

	// send recieved type and argument to Wifi_Task
	res = xQueueSend(wifi_queue_handle, &data, pdMS_TO_TICKS(50));
	if(pdPASS != res){

		// free resources if queue is full
		if(arg){
			if(heap_caps_get_allocated_size(arg)) free(arg);
		}
	}
}

/* handler for wifi events */
static void wifi_evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

	if((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_START == event_id)){

		wifi_routine_request(wifi_start, NULL);
	}

	else if((WIFI_EVENT == event_base) && (WIFI_EVENT_SCAN_DONE == event_id)){

		wifi_routine_request(wifi_scan_done, NULL);
	}

	else if((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_DISCONNECTED == event_id)){

		wifi_routine_request(wifi_disconnected, NULL);
	}

	else if((IP_EVENT == event_base) && (IP_EVENT_STA_GOT_IP == event_id)){

		wifi_routine_request(wifi_sta_got_ip, NULL);
	}
}

/* function to be performed when wifi module starts */
static void wifi_start_routine(void *arg){

	xEventGroupClearBits(WifiEvents, WIFI_CONNECTED_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_DISCONNECTED_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_AUTOCONNECT_ENABLE_BIT);
	UI_ReportEvt(UI_EVT_WIFI_DISCONNECTED, NULL);
}

/* function to be performed when scan start is requested */
static void wifi_scan_start_routine(void *arg){

	esp_err_t res;
	EventBits_t bits = xEventGroupGetBits(WifiEvents);

	// skip if scanning is in progress
	if(bits & WIFI_SCAN_IN_PROGRESS_BIT) return;
	else xEventGroupSetBits(WifiEvents, WIFI_SCAN_IN_PROGRESS_BIT);

	// start scan with default settings
	res = esp_wifi_scan_start(NULL, false);
	if(ESP_OK != res){

		ESP_LOGE("", "esp_wifi_scan_start failed, res = %d", res);
		esp_wifi_clear_ap_list();
		xEventGroupClearBits(WifiEvents, WIFI_SCAN_IN_PROGRESS_BIT);
	}
	else{

		UI_ReportEvt(UI_EVT_WIFI_LIST_CLEAR, NULL);
	}
}

/* function to be performed when scanning has been finished */
static void wifi_scan_done_routine(void *arg){

	esp_err_t res;

	// check number of found AP's
	res = esp_wifi_scan_get_ap_num(&ap_count);
	if(ESP_OK != res) goto cleanup;

	// free previously allocated resources
	if(ap_list){
		if(heap_caps_get_allocated_size(ap_list)) free(ap_list);
	}

	// allocate memory for results
	ap_list = calloc(ap_count, sizeof(wifi_ap_record_t));
	if(0 == ap_list) goto cleanup;

	// copy results
	res = esp_wifi_scan_get_ap_records(&ap_count, ap_list);
	if(ESP_OK != res) {

		free(ap_list);
		ap_list = 0;
		ap_count = 0;
		goto cleanup;
	}

	ESP_LOGI("", "%d AP's found", ap_count);

	// send results to UI
	fill_wifi_list_with_found_aps();

	cleanup:
		esp_wifi_clear_ap_list();
		xEventGroupClearBits(WifiEvents, WIFI_SCAN_IN_PROGRESS_BIT);
}

/* function called when disconnected from AP or connection to AP has failed */
static void wifi_disconnected_routine(void *arg){

	// if semaphore for reconnecting does exists
	if(0 != wifi_reconnect_semaphore_handle){

		// if possible, take semaphore and connect to wifi
		if(pdTRUE == xSemaphoreTake(wifi_reconnect_semaphore_handle, 0)){

			esp_wifi_connect();
		}
		else{
				// if no more semaphores are avalible clear resources and report connecting error
			vSemaphoreDelete(wifi_reconnect_semaphore_handle);
			wifi_reconnect_semaphore_handle = 0;
			xEventGroupClearBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
			xEventGroupClearBits(WifiEvents, WIFI_SAVE_PASSWORD_BIT);
			UI_ReportEvt(UI_EVT_WIFI_CONNECT_ERROR, NULL);
		}
	}

	// set status bits
	xEventGroupClearBits(WifiEvents, WIFI_CONNECTED_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_DISCONNECTED_BIT);
	UI_ReportEvt(UI_EVT_WIFI_DISCONNECTED, NULL);
}

static void wifi_connected_routine(void *arg){


}

/* function called when statios is connected and recieved IP from AP's DHCP */
static void wifi_sta_got_ip_routine(void *arg){

	esp_err_t e;
	int a;
	wifi_ap_record_t ap_data = {0};
	wifi_config_t wifi_config = {0};
	esp_netif_ip_info_t netif_data = {0};
	UI_DetailedAPData_t *ui_data = 0;
	WifiCreds_t *creds = 0;
	EventBits_t bits = xEventGroupGetBits(WifiEvents);

	// set status bits and clear the rest of reconnecting variables
	xEventGroupClearBits(WifiEvents, WIFI_DISCONNECTED_BIT);
	xEventGroupClearBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_CONNECTED_BIT);
	vSemaphoreDelete(wifi_reconnect_semaphore_handle);
	wifi_reconnect_semaphore_handle = 0;

	// get the data of current AP
	e = esp_wifi_sta_get_ap_info(&ap_data);
	if(ESP_OK != e) goto error;

	// get current network interface data
	e = esp_netif_get_ip_info(station_netif_obj, &netif_data);
	if(ESP_OK != e) goto error;

	// prepare output data
	ui_data = calloc(1, sizeof(UI_DetailedAPData_t));
	if(0 == ui_data) goto error;

	// copy SSID
	a = strnlen((char *)ap_data.ssid, 33);
	if((0 == a) || (33 == a)) goto error;
	ui_data->ssid = malloc(a + 1);
	if(0 == ui_data->ssid) goto error;
	memcpy(ui_data->ssid, ap_data.ssid, (a + 1));

	// copy MAC
	for(a = 0; a < 6; a++){

		ui_data->mac[a] = ap_data.bssid[a];
	}

	// copy RSSI and authetincation mode
	ui_data->rssi = ap_data.rssi;
	ui_data->mode = ap_data.authmode;

	// prepare string with IP address
	sprintf(ui_data->ip, IPSTR, IP2STR(&netif_data.gw));

	// send data to UI
	UI_ReportEvt(UI_EVT_WIFI_CONNECTED, ui_data);

	if(bits & WIFI_SAVE_PASSWORD_BIT){

		// get config of connected AP
		e = esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
		if(ESP_OK != e) goto error;

    	// prepare data to save
    	creds = calloc(1, sizeof(WifiCreds_t));
    	if(0 == creds) goto error;

    	a = strnlen((char *)wifi_config.sta.ssid, 32);
    	if((0 == a) || (32 == a)) goto error;
    	creds->ssid = malloc(a + 1);
    	if(0 == creds->ssid) goto error;
    	memcpy(creds->ssid, (char *)ap_data.ssid, a + 1);

    	a = strnlen((char *)wifi_config.sta.password, 64);
    	if((0 == a) || (64 == a)) goto error;
    	creds->pass = malloc(a + 1);
    	if(0 == creds->pass) goto error;
    	memcpy(creds->pass, (char *)wifi_config.sta.password, a + 1);

    	// save password for current network
    	SPIFFS_SavePass(creds);
		xEventGroupClearBits(WifiEvents, WIFI_SAVE_PASSWORD_BIT);
	}
	return;

	error:
		if(creds){
			if(creds->ssid){
				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
			}
			if(creds->pass){
				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
			}
			if(heap_caps_get_allocated_size(creds)) free(creds);
		}
		if(ui_data){

			if(ui_data->ssid){
				if(heap_caps_get_allocated_size(ui_data->ssid)) free(ui_data->ssid);
			}
			if(heap_caps_get_allocated_size(ui_data)) free(ui_data);
		}
		UI_ReportEvt(UI_EVT_WIFI_CONNECTED, NULL);
}

/* function called when ssid and password are known and connection try should be performed */
static void wifi_connect_routine(void *arg){

	int a;
	WifiCreds_t *creds = (WifiCreds_t *)arg;
	wifi_config_t wifi_config = {0};
	EventBits_t bits = xEventGroupGetBits(WifiEvents);

	// return if already trying to connect
	if(bits & WIFI_CONNECTION_IN_PROGRESS_BIT) goto cleanup;

	// copy ssid to config structure
	a = strnlen(creds->ssid, 31);
	if((0 == a) || (31 == a)) goto cleanup;
	memcpy(wifi_config.sta.ssid, creds->ssid, (a + 1));

	// copy password to config structure
	a = strnlen(creds->pass, 63);
	if((0 == a) || (63 == a)) goto cleanup;
	memcpy(wifi_config.sta.password, creds->pass, (a + 1));

	// prepare semaphores for reconnecting
	if(0 != wifi_reconnect_semaphore_handle) vSemaphoreDelete(wifi_reconnect_semaphore_handle);
	wifi_reconnect_semaphore_handle = xSemaphoreCreateCounting(WIFI_RECONNECT_ATTEMPTS, WIFI_RECONNECT_ATTEMPTS);
	if(0 == wifi_reconnect_semaphore_handle) goto cleanup;

	// set config and connect
	if(ESP_OK != esp_wifi_set_config(WIFI_IF_STA, &wifi_config)) goto cleanup;
//	if(ESP_OK != esp_wifi_connect()) goto cleanup;

	// set stataus bits
	UI_ReportEvt(UI_EVT_WIFI_CONNECTING, arg);
	xEventGroupSetBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
	if(true == creds->save){

		// set this bit if user requested to save password (UI)
		xEventGroupSetBits(WifiEvents, WIFI_SAVE_PASSWORD_BIT);
	}

	// disconnect current AP if connected
	// "disconnected" event should call "connect" function
	if(bits & WIFI_CONNECTED_BIT) {

		esp_wifi_disconnect();
	}
	else{

		esp_wifi_connect();
	}
	return;

	cleanup:
		free(creds->ssid);
		free(creds->pass);
		free(creds);
}

/* send all found AP's to Wifi list on WifiScreen */
static void fill_wifi_list_with_found_aps(void){

	uint8_t i;
	UI_BasicAPData_t *data;

	for(i = 0; i < ap_count; i++){

		data = calloc(1U, sizeof(UI_BasicAPData_t));
		if(0 == data) return;

		if(WIFI_AUTH_OPEN == ap_list[i].authmode) data->is_protected = false;
		else data->is_protected = true;
		data->ssid = (char *)ap_list[i].ssid;
		data->rssi = ap_list[i].rssi;

		UI_ReportEvt(UI_EVT_WIFI_LIST_ADD, data);
	}
}
