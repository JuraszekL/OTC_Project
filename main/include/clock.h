#ifndef MAIN_INCLUDE_CLOCK_H_
#define MAIN_INCLUDE_CLOCK_H_

#define BASIC_DATA_RESFRESH_MINUTE		17

void Clock_Update(int TimeUnix, const char *TimezoneString);
void Clock_Task(void *arg);

#endif /* MAIN_INCLUDE_CLOCK_H_ */
