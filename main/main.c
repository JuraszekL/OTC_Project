#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "main.h"
#include "display.h"
#include "touchpad.h"
#include "ui_task.h"

EventGroupHandle_t AppStartSyncEvt;
const char tag[] = "main.c";

void app_main(void){

	// initialize hardware for peripherials
	Disp_PeriphInit();
	TouchPad_PeriphInit();

	// create synchronization point
	AppStartSyncEvt = xEventGroupCreate();
	if(NULL == AppStartSyncEvt){

		ESP_LOGE(tag, "xEventGroupCreate returned NULL");
		esp_restart();
	}

	// create the tasks
	xTaskCreate(Display_Task, "", 8192, NULL, 1, NULL);
	xTaskCreate(TouchPad_Task, "", 4096, NULL, 1, NULL);
	xTaskCreate(UI_Task, "", 8192, NULL, 1, NULL);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, MAIN_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	// delete resources and kill app_main task
	vEventGroupDelete(AppStartSyncEvt);
	vTaskDelete(NULL);
}
