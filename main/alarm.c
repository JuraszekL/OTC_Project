#include "main.h"
#include <string.h>

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static AlarmData_t* alarm_get_copy(AlarmData_t *org_alarm);
static void alarm_play(uint8_t idx);

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

	res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE != res) return 0;

	alarm = alarm_get_copy(alarms[idx]);

	xSemaphoreGive(alarm_mutex_handle);
	return alarm;
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

/* set new status for selected alarm */
void Alarm_SetStatus(uint8_t idx, bool status){

	if((idx >= ALARMS_NUMBER)) return;

	AlarmData_t *alarm;
	BaseType_t res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE == res){

		if(alarms[idx]->status != status){

			alarms[idx]->status = status;

			alarm = alarm_get_copy(alarms[idx]);

			SPIFFS_UpdateAlarm(idx, alarm);
		}

		xSemaphoreGive(alarm_mutex_handle);
	}
}

/* set new values for selected alarm */
void Alarm_SetValues(uint8_t idx, AlarmData_t *alarm){

	if((0 == alarm) || (idx >= ALARMS_NUMBER)) goto error;
	if((24U <= alarm->hour) || (60U <= alarm->minute) || (0 == alarm->text)) goto error;

	AlarmData_t *current_values, *copy;
	BaseType_t res;

	res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE != res) goto error;

	// store the old structure's pointer and replace it with the new one
	current_values = alarms[idx];
	alarms[idx] = alarm;

	// copy the new structure and store it to SPIFFS
	copy = alarm_get_copy(alarms[idx]);
	SPIFFS_UpdateAlarm(idx, copy);

	// free the old one structure
	if(current_values->text){
		if(heap_caps_get_allocated_size(current_values->text)) free(current_values->text);
	}
	if(heap_caps_get_allocated_size(current_values)) free(current_values);

	xSemaphoreGive(alarm_mutex_handle);
	return;

	error:
		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}
}

/* check all alarms if should be played (function called by clock task every minute) */
void Alarm_CheckRoutine(uint8_t weekday, uint8_t hour, uint8_t minute){

	uint8_t a;
	BaseType_t res;

	res = xSemaphoreTake(alarm_mutex_handle, pdMS_TO_TICKS(50));
	if(pdTRUE != res) return;

	for(a = 0; a < ALARMS_NUMBER; a++){

		if(true == alarms[a]->status){		// if alarm is active

			if((alarms[a]->hour == hour) && (alarms[a]->minute == minute)){		// if hour and minute fit

				if((alarms[a]->flags & (1 << weekday)) || (0 == alarms[a]->flags)){		// if the day of repeat fits or a single alarm is set

					alarm_play(a);
				}
			}
		}
	}

	xSemaphoreGive(alarm_mutex_handle);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* get a copy of an AlarmData_t structure */
static AlarmData_t* alarm_get_copy(AlarmData_t *org_alarm){

	AlarmData_t *return_alarm;
	int a;

	return_alarm = calloc(1U, sizeof(AlarmData_t));
	if(0 == return_alarm) goto error;

	return_alarm->hour = org_alarm->hour;
	return_alarm->minute = org_alarm->minute;
	return_alarm->flags = org_alarm->flags;
	return_alarm->status = org_alarm->status;

	a = strlen(org_alarm->text);
	return_alarm->text = malloc(a + 1);
	if(0 == return_alarm->text) goto error;
	memcpy(return_alarm->text, org_alarm->text, (a + 1));

	return return_alarm;

	error:
		if(return_alarm){
			if(return_alarm->text){
				if(heap_caps_get_allocated_size(return_alarm->text)) free(return_alarm->text);
			}
			if(heap_caps_get_allocated_size(return_alarm)) free(return_alarm);
		}
		return 0;
}

/* deactivate if one time alarm, and run UI to play it */
static void alarm_play(uint8_t idx){

	AlarmData_t *alarm;
	uint32_t tmp = idx;

	if(0 == alarms[idx]->flags){

		// if one time alarm needs to be played, it should be deactivated as well
		alarms[idx]->status = false;
		alarm = alarm_get_copy(alarms[idx]);
		SPIFFS_UpdateAlarm(idx, alarm);
		UI_ReportEvt(UI_EVT_ALARM_VALUES_CHANGED, (void *)tmp);
	}

	UI_ReportEvt(UI_EVT_ALARM_RUN, (void *)tmp);
}
