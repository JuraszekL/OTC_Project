#include "main.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "cJSON.h"

static void one_second_timer_callback(TimerHandle_t xTimer);

EventGroupHandle_t AppStartSyncEvt;
TimerHandle_t OneSecondTimer;

const char tag[] = "main.c";

void app_main(void){

	esp_err_t ret;
	cJSON_Hooks hooks = {

			.malloc_fn = lvgl_malloc,
			.free_fn = free,
	};

	// set functions for json library
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

	// create the tasks for core 1
	xTaskCreatePinnedToCore(Wifi_Task, "WiFi_Task", 4096, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(Clock_Task, "Clock_Task", 4096, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(OnlineRequests_Task, "OnlineRequests_Task", 4096, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(SPIFFS_Task, "SPIFFS_Task", 8192, NULL, 1, NULL, 1);

	// create the tasks for core 0
	xTaskCreatePinnedToCore(Display_Task, "Display_Task", 8192, NULL, 3, NULL, 0);
	xTaskCreatePinnedToCore(TouchPad_Task, "TouchPad_Task", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(UI_Task, "UI_Task", 8192, NULL, 2, NULL, 0);
	xTaskCreatePinnedToCore(SDCard_Task, "SDCard_Task", 8192, NULL, 1, NULL, 0);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, MAIN_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	OneSecondTimer = xTimerCreate("", pdMS_TO_TICKS(1000), pdFALSE, NULL, one_second_timer_callback);
	assert(OneSecondTimer);

	// delete resources and kill app_main task
	vEventGroupDelete(AppStartSyncEvt);
	vTaskDelete(NULL);
}

/* wrappers to use external RAM for LVGL and JSON */
void * IRAM_ATTR lvgl_malloc(size_t size){

	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void IRAM_ATTR lvgl_free(void * data){

	free(data);
}

void * IRAM_ATTR lvgl_realloc(void * data_p, size_t new_size){

	return heap_caps_realloc(data_p, new_size, MALLOC_CAP_SPIRAM);
}

static void one_second_timer_callback(TimerHandle_t xTimer){

}
