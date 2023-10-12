#ifndef MAIN_INCLUDE_CLOCK_H_
#define MAIN_INCLUDE_CLOCK_H_

/**************************************************************
 * Public functions
 ***************************************************************/
void Clock_Task(void *arg);

void TimeUpdated(void);
void Timezone_Update(const char *TimezoneString);

#endif /* MAIN_INCLUDE_CLOCK_H_ */
