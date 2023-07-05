#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "main.h"
#include "display.h"
#include "touchpad.h"
#include "ui.h"

EventGroupHandle_t AppStartSyncEvt;
const char tag[] = "main.c";

void app_main(void){

	AppStartSyncEvt = xEventGroupCreate();
	if(NULL == AppStartSyncEvt){

		ESP_LOGE(tag, "xEventGroupCreate returned NULL");
		esp_restart();
	}

	Disp_PeriphInit();
	TouchPad_PeriphInit();

	xTaskCreate(Display_Task, "", 4096, NULL, 1, NULL);
	xTaskCreate(TouchPad_Task, "", 4096, NULL, 1, NULL);

//	xEventGroupSync(AppStartSyncEvt, MAIN_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

    	vTaskDelay(pdMS_TO_TICKS(1000));

    }
}
