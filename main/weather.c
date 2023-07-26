#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "ui.h"

#include "main.h"
#include "wifi.h"
#include "online_requests.h"
#include "weather.h"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef void (*weather_evt)(void *arg);

struct weather_evt_queue_data {

	Weather_EventType_t type;
	void *arg;
};

struct weather_icon {

	int code;
	const lv_img_dsc_t *day_icon;
	const lv_img_dsc_t *night_icon;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
void weather_simple_update(void *arg);
void weather_detailed_update(void *arg);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static const weather_evt events_tab[] = {

		[WEATHER_SIMPLE_UPDATE] = weather_simple_update,
		[WEATHER_DETAILED_UPDATE] = weather_detailed_update,
};

static const struct weather_icon icons[] = {

		{1000, &ui_img_day_113_png, &ui_img_night_113_png},
		{1003, &ui_img_day_116_png, &ui_img_night_116_png},
		{1006, &ui_img_day_119_png, &ui_img_night_119_png},
		{1030, &ui_img_day_143_png, &ui_img_night_143_png},
		{1063, &ui_img_day_176_png, &ui_img_night_176_png},
		{1066, &ui_img_day_179_png, &ui_img_night_179_png},
		{1069, &ui_img_day_182_png, &ui_img_night_182_png},
		{1072, &ui_img_day_185_png, &ui_img_night_185_png},
		{1087, &ui_img_day_200_png, &ui_img_night_200_png},
		{1114, &ui_img_day_227_png, &ui_img_night_227_png},
		{1117, &ui_img_day_230_png, &ui_img_night_230_png},
		{1135, &ui_img_day_248_png, &ui_img_night_248_png},
		{1147, &ui_img_day_260_png, &ui_img_night_260_png},
		{1150, &ui_img_day_263_png, &ui_img_night_263_png},
		{1153, &ui_img_day_266_png, &ui_img_night_266_png},
		{1168, &ui_img_day_281_png, &ui_img_night_281_png},
		{1171, &ui_img_day_284_png, &ui_img_night_284_png},
		{1180, &ui_img_day_293_png, &ui_img_night_293_png},
		{1183, &ui_img_day_296_png, &ui_img_night_296_png},
		{1186, &ui_img_day_299_png, &ui_img_night_299_png},
		{1189, &ui_img_day_302_png, &ui_img_night_302_png},
		{1192, &ui_img_day_305_png, &ui_img_night_305_png},
		{1195, &ui_img_day_308_png, &ui_img_night_308_png},
		{1198, &ui_img_day_311_png, &ui_img_night_311_png},
		{1201, &ui_img_day_314_png, &ui_img_night_314_png},
		{1204, &ui_img_day_317_png, &ui_img_night_317_png},
		{1207, &ui_img_day_320_png, &ui_img_night_320_png},
		{1210, &ui_img_day_323_png, &ui_img_night_323_png},
		{1213, &ui_img_day_326_png, &ui_img_night_326_png},
		{1216, &ui_img_day_329_png, &ui_img_night_329_png},
		{1219, &ui_img_day_332_png, &ui_img_night_332_png},
		{1222, &ui_img_day_335_png, &ui_img_night_335_png},
		{1225, &ui_img_day_338_png, &ui_img_night_338_png},
		{1237, &ui_img_day_350_png, &ui_img_night_350_png},
		{1240, &ui_img_day_353_png, &ui_img_night_353_png},
		{1243, &ui_img_day_356_png, &ui_img_night_356_png},
		{1246, &ui_img_day_359_png, &ui_img_night_359_png},
		{1249, &ui_img_day_362_png, &ui_img_night_362_png},
		{1249, &ui_img_day_362_png, &ui_img_night_362_png},
		{1252, &ui_img_day_365_png, &ui_img_night_365_png},
		{1255, &ui_img_day_368_png, &ui_img_night_368_png},
		{1258, &ui_img_day_371_png, &ui_img_night_371_png},
		{1261, &ui_img_day_374_png, &ui_img_night_374_png},
		{1264, &ui_img_day_377_png, &ui_img_night_377_png},
		{1273, &ui_img_day_386_png, &ui_img_night_386_png},
		{1276, &ui_img_day_389_png, &ui_img_night_389_png},
		{1279, &ui_img_day_392_png, &ui_img_night_392_png},
		{1282, &ui_img_day_395_png, &ui_img_night_395_png},
		{0, 0, 0},
};

static QueueHandle_t weather_evt_queue_handle;


/******************************************************************************************************************
 *
 * Weather Event task
 *
 ******************************************************************************************************************/
void Weather_Task(void *arg){

	BaseType_t ret;
	struct weather_evt_queue_data data;

	// create online requests queue
	weather_evt_queue_handle = xQueueCreate(3U, sizeof(struct weather_evt_queue_data));
	assert(weather_evt_queue_handle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, WEATHER_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// wait for new events
		ret = xQueueReceive(weather_evt_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			events_tab[data.type](data.arg);
		}
//		vTaskDelay(pdMS_TO_TICKS(10000));
//		OnlineRequest_Send(ONLINEREQ_WEATHER_UPDATE, NULL);
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* request online data */
void Weather_EventReport(Weather_EventType_t Type, void *arg){

	struct weather_evt_queue_data data;

	data.type = Type;
	data.arg = arg;

	// send data to requests queue in OnlineRequests_Task
	xQueueSend(weather_evt_queue_handle, &data, pdMS_TO_TICKS(50));
}

void weather_simple_update(void *arg){

	int a;
	Weather_SimpleData_t *data = (Weather_SimpleData_t *)arg;

//	ESP_LOGI("", "is_day = %d, weather_code = %d", data->is_day, data->weather_code);

	for(a = 0; 0 != icons[a].code; a++){

		if(icons[a].code == data->weather_code){

			if(data->is_day){

				lv_img_set_src(ui_WeatherImage, icons[a].day_icon);
			}
			else{

				lv_img_set_src(ui_WeatherImage, icons[a].night_icon);
			}
		}
	}
}

void weather_detailed_update(void *arg){


}
