#include "main.h"
#include <sys/time.h>

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static AlarmData_t *alarms[4];

static SemaphoreHandle_t alarm_mutex_handle;

static const AlarmData_t default_alarm = {

		.hour = 0,
		.minute = 0,
		.flags = 0,
		.status = false,
		.text = "-- not-set --",
};

/**************************************************************
 *
 *	Public functions
 *
 ***************************************************************/

/* init mutex */
void Alarm_InitResources(void){

	alarm_mutex_handle = xSemaphoreCreateMutex();
	assert(alarm_mutex_handle);
}

/* return the structure with default values */
const AlarmData_t* Alarm_GetDefaultValues(void){

	return &default_alarm;
}

/* restores alarm from SPIFFS (at startup) */
void Alarm_RestoreFromSPIFFS(AlarmData_t *alarm, uint8_t idx){

	if((0 == alarm) || (idx >= ALARMS_NUMBER)) return;

	BaseType_t res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE == res){

		alarms[idx] = alarm;
		xSemaphoreGive(alarm_mutex_handle);
	}
}
