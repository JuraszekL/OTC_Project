#ifndef MAIN_INCLUDE_WIFI_H_
#define MAIN_INCLUDE_WIFI_H_

#define WIFI_CONNECTED_BIT			(1U << 0)
#define WIFI_DISCONNECTED_BIT		(1U << 1)

extern TaskHandle_t Wifi_TaskHandle;;
extern EventGroupHandle_t WifiEvents;

void Wifi_Task(void *arg);


#endif /* MAIN_INCLUDE_WIFI_H_ */
