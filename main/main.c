#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "main.h"
#include "display.h"
#include "touchpad.h"
#include "ui_task.h"
#include "wifi.h"

EventGroupHandle_t AppStartSyncEvt;
const char tag[] = "main.c";

void app_main(void){

	esp_err_t ret;

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
	if(NULL == AppStartSyncEvt){

		ESP_LOGE(tag, "xEventGroupCreate returned NULL");
		esp_restart();
	}

	// create the tasks fo core 0
	xTaskCreatePinnedToCore(Wifi_Task, "", 8192, NULL, 1, NULL, 0);

	// create the tasks fo core 1
	xTaskCreatePinnedToCore(Display_Task, "", 8192, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(TouchPad_Task, "", 4096, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(UI_Task, "", 8192, NULL, 2, NULL, 1);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, MAIN_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// delete resources and kill app_main task
	vEventGroupDelete(AppStartSyncEvt);
	vTaskDelete(NULL);
}
