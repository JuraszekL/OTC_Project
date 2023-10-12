
#ifndef MAIN_INCLUDE_ONLINE_REQUESTS_H_
#define MAIN_INCLUDE_ONLINE_REQUESTS_H_

/**************************************************************
 * Online request types
 ***************************************************************/
typedef enum {

	ONLINEREQ_BASIC_UPDATE = 0,
	ONLINEREQ_TIME_UPDATE,
	ONLINEREQ_DETAILED_UPDATE,

} OnlineRequest_Type_t;

/**************************************************************
 * Public functions
 ***************************************************************/
void OnlineRequests_Task(void *arg);

void OnlineRequest_Send(OnlineRequest_Type_t Type, void *arg);

#endif /* MAIN_INCLUDE_ONLINE_REQUESTS_H_ */
