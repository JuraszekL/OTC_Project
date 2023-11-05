#ifndef MAIN_INCLUDE_ALARM_H_
#define MAIN_INCLUDE_ALARM_H_

/**************************************************************
 * Definitions
 ***************************************************************/
#define ALARMS_NUMBER				4U

// useless definitions, left only for information how weekdays are represented by bits

//#define ALARM_SUNDAY_BIT			0U
//#define ALARM_MONDAY_BIT			1U
//#define ALARM_TUESDAY_BIT			2U
//#define ALARM_WEDNESDAY_BIT		3U
//#define ALARM_THURSDAY_BIT		4U
//#define ALARM_FRIDAY_BIT			5U
//#define ALARM_SATURDAY_BIT		6U

/**************************************************************
 * Public functions
 ***************************************************************/
void Alarm_InitResources(void);
const AlarmData_t* Alarm_GetDefaultValues(void);
AlarmData_t* Alarm_GetCurrentValues(uint8_t idx);
void Alarm_RestoreFromSPIFFS(AlarmData_t *alarm, uint8_t idx);
void Alarm_SetStatus(uint8_t idx, bool status);
void Alarm_SetValues(uint8_t idx, AlarmData_t *alarm);
void Alarm_CheckRoutine(uint8_t weekday, uint8_t hour, uint8_t minute);

#endif /* MAIN_INCLUDE_ALARM_H_ */
