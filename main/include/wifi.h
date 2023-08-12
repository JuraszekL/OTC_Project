#ifndef MAIN_INCLUDE_WIFI_H_
#define MAIN_INCLUDE_WIFI_H_

/**************************************************************
 * WIFI status bits
 ***************************************************************/


/**************************************************************
 * Public variables
 ***************************************************************/
extern SemaphoreHandle_t WifiList_MutexHandle;;

/**************************************************************
 * Public functions
 ***************************************************************/
void Wifi_Task(void *arg);
bool Wifi_WaitUntilIsConnected(unsigned int Time_ms);
void WIFI_StartScan(void);

#endif /* MAIN_INCLUDE_WIFI_H_ */
