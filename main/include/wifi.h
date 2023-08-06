#ifndef MAIN_INCLUDE_WIFI_H_
#define MAIN_INCLUDE_WIFI_H_

/**************************************************************
 * WIFI status bits
 ***************************************************************/
#define WIFI_CONNECTED_BIT			(1U << 0)
#define WIFI_DISCONNECTED_BIT		(1U << 1)

/**************************************************************
 * Public variables
 ***************************************************************/
extern TaskHandle_t Wifi_TaskHandle;;
extern EventGroupHandle_t WifiEvents;

/**************************************************************
 * Public functions
 ***************************************************************/
void Wifi_Task(void *arg);

#endif /* MAIN_INCLUDE_WIFI_H_ */
