#ifndef MAIN_INCLUDE_CLOCK_H_
#define MAIN_INCLUDE_CLOCK_H_

/**************************************************************
 * Minute to refresh time and basic weather
 ***************************************************************/
#define BASIC_DATA_RESFRESH_MINUTE		37

/**************************************************************
 * Public functions
 ***************************************************************/
void TimeUpdated(void);
void Timezone_Update(const char *TimezoneString);
void Clock_Task(void *arg);

#endif /* MAIN_INCLUDE_CLOCK_H_ */
