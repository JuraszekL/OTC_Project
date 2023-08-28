#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gptimer.h"
#include "esp_wifi.h"

#include "main.h"
#include "ui_task.h"
#include "spiffs_task.h"
#include "wifi.h"

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

/* other definitions */
#define WIFI_RECONNECT_ATTEMPTS				5

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
 ***************************************************************/
SemaphoreHandle_t WifiList_MutexHandle;

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

	// mutex for Wifi operations
	WifiList_MutexHandle = xSemaphoreCreateMutex();
	assert(WifiList_MutexHandle);

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

	if(0 == creds->pass){

		SPIFFS_GetPass(creds);
	}
	else{

		wifi_routine_request(wifi_connect, creds);
	}
}

/* function called by filesystem when searching for password has been finished
 * if no password found, creds->pass == 0, and function runs UI to obtain password from user */
void Wifi_ReportPass(WifiCreds_t *creds){

	if((0 == creds) || (0 == creds->ssid)) return;

	if(0 == creds->pass){

//		/UI_ReportEvt(UI_EVT_WIFI_GET_PASSWORD, creds);
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
//	wifi_routine_request(wifi_scan_start, NULL);
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
	BaseType_t a;

	// if nothing else uses wifi resources
	a = xSemaphoreTake(WifiList_MutexHandle, pdMS_TO_TICKS(1000));
	if(pdFALSE == a) goto cleanup;

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
		xSemaphoreGive(WifiList_MutexHandle);
}

static void wifi_disconnected_routine(void *arg){

	if(0 != wifi_reconnect_semaphore_handle){

		if(pdTRUE == xSemaphoreTake(wifi_reconnect_semaphore_handle, 0)){

			esp_wifi_connect();
		}
		else{

			vSemaphoreDelete(wifi_reconnect_semaphore_handle);
			wifi_reconnect_semaphore_handle = 0;
			xEventGroupClearBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
			// UI_Report_connectionfailed
		}
	}

	xEventGroupClearBits(WifiEvents, WIFI_CONNECTED_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_DISCONNECTED_BIT);
	UI_ReportEvt(UI_EVT_WIFI_DISCONNECTED, NULL);
}

static void wifi_connected_routine(void *arg){


}

static void wifi_sta_got_ip_routine(void *arg){

	xEventGroupClearBits(WifiEvents, WIFI_DISCONNECTED_BIT);
	xEventGroupClearBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
	xEventGroupSetBits(WifiEvents, WIFI_CONNECTED_BIT);
	UI_ReportEvt(UI_EVT_WIFI_CONNECTED, NULL);

	vSemaphoreDelete(wifi_reconnect_semaphore_handle);
	wifi_reconnect_semaphore_handle = 0;
}

static void wifi_connect_routine(void *arg){

	int a;
	WifiCreds_t *creds = (WifiCreds_t *)arg;
	wifi_config_t wifi_config = {0};
	EventBits_t bits = xEventGroupGetBits(WifiEvents);

	if(bits & WIFI_CONNECTION_IN_PROGRESS_BIT) goto cleanup;

	a = strnlen(creds->ssid, 31);
	if((0 == a) || (31 == a)) goto cleanup;
	memcpy(wifi_config.sta.ssid, creds->ssid, (a + 1));

	a = strnlen(creds->pass, 31);
	if((0 == a) || (63 == a)) goto cleanup;
	memcpy(wifi_config.sta.password, creds->pass, (a + 1));

	if(0 != wifi_reconnect_semaphore_handle) vSemaphoreDelete(wifi_reconnect_semaphore_handle);
	wifi_reconnect_semaphore_handle = xSemaphoreCreateCounting(WIFI_RECONNECT_ATTEMPTS, WIFI_RECONNECT_ATTEMPTS);
	if(0 == wifi_reconnect_semaphore_handle) goto cleanup;

	if(ESP_OK != esp_wifi_set_config(WIFI_IF_STA, &wifi_config)) goto cleanup;
	if(ESP_OK != esp_wifi_connect()) goto cleanup;

	UI_ReportEvt(UI_EVT_WIFI_CONNECTING, arg);
	xEventGroupSetBits(WifiEvents, WIFI_CONNECTION_IN_PROGRESS_BIT);
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
