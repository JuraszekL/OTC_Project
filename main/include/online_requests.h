
#ifndef MAIN_INCLUDE_ONLINE_REQUESTS_H_
#define MAIN_INCLUDE_ONLINE_REQUESTS_H_

#define ONLINE_REQ_WAIT_WIFI_MS		1000

#define SNTP_SERVER_NAME			"ntp1.tp.pl"

typedef enum {

	ONLINEREQ_CLOCK_UPDATE = 0,
	ONLINEREQ_TIMEZONE_UPDATE,
	ONLINEREQ_WEATHER_UPDATE,

} OnlineRequest_Type_t;

void OnlineRequest_Send(OnlineRequest_Type_t Type, void *arg);

void OnlineRequests_Task(void *arg);


#endif /* MAIN_INCLUDE_ONLINE_REQUESTS_H_ */
