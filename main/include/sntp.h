#ifndef MAIN_INCLUDE_SNTP_H_
#define MAIN_INCLUDE_SNTP_H_

#define SNTP_SERVER_NAME				"ntp1.tp.pl"

#define SNTP_REFRESH_HOUR		22
#define SNTP_RESFRESH_MINUTE	36

void SNTP_Task(void *arg);

#endif /* MAIN_INCLUDE_SNTP_H_ */
