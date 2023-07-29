#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "cJSON.h"

#include "main.h"
#include "display.h"
#include "touchpad.h"
#include "ui_task.h"
#include "wifi.h"
#include "clock.h"
#include "online_requests.h"
#include "weather.h"

void * cJson_Malloc(size_t sz);

EventGroupHandle_t AppStartSyncEvt;
const char tag[] = "main.c";

void app_main(void){

	esp_err_t ret;
	cJSON_Hooks hooks = {

			.malloc_fn = cJson_Malloc,
			.free_fn = free,
	};

	cJSON_InitHooks(&hooks);

    // Initialize NVS
	ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

	// initialize hardware for peripherials
	Disp_PeriphInit();
	TouchPad_PeriphInit();

	// create synchronization point
	AppStartSyncEvt = xEventGroupCreate();
	assert(AppStartSyncEvt);

	// create the tasks fo core 0
	xTaskCreatePinnedToCore(Wifi_Task, "WiFi_Task", 8192, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(Clock_Task, "Clock_Task", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(OnlineRequests_Task, "OnlineRequests_Task", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(Weather_Task, "Weather_Task", 4096, NULL, 1, NULL, 0);

	// create the tasks fo core 1
	xTaskCreatePinnedToCore(Display_Task, "Display_Task", 8192, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(TouchPad_Task, "TouchPad_Task", 4096, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(UI_Task, "UI_Task", 8192, NULL, 2, NULL, 1);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, MAIN_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// delete resources and kill app_main task
	vEventGroupDelete(AppStartSyncEvt);
	vTaskDelete(NULL);
}

void * IRAM_ATTR cJson_Malloc(size_t sz){

	return heap_caps_calloc(sizeof(size_t), sz, MALLOC_CAP_SPIRAM);
}
