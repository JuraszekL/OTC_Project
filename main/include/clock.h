#ifndef MAIN_INCLUDE_CLOCK_H_
#define MAIN_INCLUDE_CLOCK_H_

#define CLOCK_REFRESH_HOUR			22
#define CLOCK_RESFRESH_MINUTE		36

void Clock_TimeUpdated(void);

void Clock_Task(void *arg);

#endif /* MAIN_INCLUDE_CLOCK_H_ */
