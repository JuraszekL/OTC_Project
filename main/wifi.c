#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gptimer.h"

#include "main.h"
#include "esp_wifi.h"

static void network_init(void);
static void wifi_evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

TaskHandle_t Wifi_TaskHandle;

static esp_netif_t *station_netif_obj;
static wifi_config_t wifi_config = {

		.sta = {

				.ssid = CONFIG_ESP_WIFI_SSID,
				.password = CONFIG_ESP_WIFI_PASSWORD,
		},
};


void Wifi_Task(void *arg){

	uint32_t notification_value;

	// initialize WiFi and NETIF
	network_init();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, WIFI_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// wait for the moment when nothing happens on the screen
	notification_value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	if(0 == notification_value){

		abort();
	}

	// start wifi
	ESP_ERROR_CHECK(esp_wifi_start() );

	while(1){

		// wait for wifi event to reconnect
		notification_value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(0 != notification_value){

			vTaskDelay(pdMS_TO_TICKS(3000));
			esp_wifi_connect();
		}
	}
}


static void network_init(void){

	wifi_init_config_t sta_init_config = WIFI_INIT_CONFIG_DEFAULT();

	// set task handle for external use
	Wifi_TaskHandle = xTaskGetCurrentTaskHandle();

	// initialize network interface in tcp/ip stack
	ESP_ERROR_CHECK(esp_netif_init());

	// create netif object with typical wifi station configuration
	station_netif_obj = esp_netif_create_default_wifi_sta();
	assert(station_netif_obj);

	// initialize wifi
	ESP_ERROR_CHECK(esp_wifi_init(&sta_init_config));

	// set device as station
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	// configure wifi
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

	// install wifi_evt_handler function as a handler for needed wifi events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, wifi_evt_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_evt_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_evt_handler, NULL, NULL));
}

/* handler for wifi events */
static void wifi_evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

	if((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_START == event_id)){

		xTaskNotifyGive(Wifi_TaskHandle);

	}
	else if((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_DISCONNECTED == event_id)){

		xTaskNotifyGive(Wifi_TaskHandle);
	}

	else if((IP_EVENT == event_base) && (IP_EVENT_STA_GOT_IP == event_id)){

		// coś
	}

}

