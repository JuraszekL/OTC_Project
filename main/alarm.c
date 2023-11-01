#include "main.h"
#include <string.h>
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

/* return a copy of selected alarm's structure */
AlarmData_t* Alarm_GetCurrentValues(uint8_t idx){

	if((idx >= ALARMS_NUMBER)) return 0;
	if((0 == alarms[idx])) return 0;

	AlarmData_t *alarm;
	BaseType_t res;
	int a;

	alarm = calloc(1U, sizeof(AlarmData_t));
	if(0 == alarm) goto error;

	res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE != res) goto error;

	alarm->hour = alarms[idx]->hour;
	alarm->minute = alarms[idx]->minute;
	alarm->flags = alarms[idx]->flags;
	alarm->status = alarms[idx]->status;

	a = strlen(alarms[idx]->text);
	alarm->text = malloc(a + 1);
	if(0 == alarm->text) goto error;
	memcpy(alarm->text, alarms[idx]->text, (a + 1));

	xSemaphoreGive(alarm_mutex_handle);
	return alarm;

	error:
		xSemaphoreGive(alarm_mutex_handle);
		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}
		return 0;
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
