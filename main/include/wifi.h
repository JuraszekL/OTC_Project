#ifndef MAIN_INCLUDE_WIFI_H_
#define MAIN_INCLUDE_WIFI_H_

/**************************************************************
 * Public functions
 ***************************************************************/
void Wifi_Task(void *arg);

bool Wifi_WaitUntilIsConnected(unsigned int Time_ms);
void WIFI_StartScan(void);
void Wifi_Connect(WifiCreds_t *creds);

#endif /* MAIN_INCLUDE_WIFI_H_ */
