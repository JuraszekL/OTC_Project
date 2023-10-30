#ifndef MAIN_INCLUDE_ALARM_H_
#define MAIN_INCLUDE_ALARM_H_

/**************************************************************
 * Definitions
 ***************************************************************/
#define ALARMS_NUMBER			4

/**************************************************************
 * Public functions
 ***************************************************************/
void Alarm_InitResources(void);
const AlarmData_t* Alarm_GetDefaultValues(void);
void Alarm_RestoreFromSPIFFS(AlarmData_t *alarm, uint8_t idx);

#endif /* MAIN_INCLUDE_ALARM_H_ */
