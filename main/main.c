#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "touchpad.h"

void app_main(void){

	Disp_PeriphInit();
	TouchPad_PeriphInit();

	xTaskCreate(Display_Task, "", 4096, NULL, 1, NULL);
	xTaskCreate(TouchPad_Task, "", 4096, NULL, 1, NULL);

	while(1){

    	vTaskDelay(pdMS_TO_TICKS(1000));

    }
}
