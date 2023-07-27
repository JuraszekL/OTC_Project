#ifndef MAIN_INCLUDE_WEATHER_H_
#define MAIN_INCLUDE_WEATHER_H_

typedef enum {

	WEATHER_BASIC_UPDATE = 0,
	WEATHER_DETAILED_UPDATE

} Weather_EventType_t;

typedef struct {

	uint8_t is_day;
	int weather_code;

} Weather_BasicData_t;

void Weather_EventReport(Weather_EventType_t Type, void *arg);
void Weather_Task(void *arg);

#endif /* MAIN_INCLUDE_WEATHER_H_ */
