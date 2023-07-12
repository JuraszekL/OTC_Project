#ifndef MAIN_INCLUDE_SNTP_H_
#define MAIN_INCLUDE_SNTP_H_

#define SNTP_SERVER_NAME				"ntp1.tp.pl"
#define SNTP_TIME_REFRESH_PERIOD_SEC	3600

void SNTP_Task(void *arg);

#endif /* MAIN_INCLUDE_SNTP_H_ */
