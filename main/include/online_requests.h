
#ifndef MAIN_INCLUDE_ONLINE_REQUESTS_H_
#define MAIN_INCLUDE_ONLINE_REQUESTS_H_

#define HTTP_WEATHER_URL				"http://api.weatherapi.com/v1"
#define HTTP_WEATHER_METH_NOW			"/current.json?"
#define HTTP_WEATHER_METH_FORECAST		"/forecast.json?"
#define HTTP_WEATHER_PAR_KEY			"key="
#define HTTP_WEATHER_QUERY				"&q=auto:ip"

#define SNTP_SERVER_NAME				"ntp1.tp.pl"

typedef enum {

	ONLINEREQ_BASIC_UPDATE = 0,
	ONLINEREQ_TIME_UPDATE,
	ONLINEREQ_DETAILED_UPDATE,

} OnlineRequest_Type_t;

void OnlineRequest_Send(OnlineRequest_Type_t Type, void *arg);
void OnlineRequests_Task(void *arg);


#endif /* MAIN_INCLUDE_ONLINE_REQUESTS_H_ */
