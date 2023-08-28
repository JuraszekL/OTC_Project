#ifndef MAIN_UI_UI_WIFI_LIST_H_
#define MAIN_UI_UI_WIFI_LIST_H_

#include "main.h"

void UI_WifiListInit(void);
void UI_WifiListClear(void);
void UI_WifiListAdd(bool is_protected, char *name, int rssi);
void UI_WifiPopup_Connecting(char *ssid);
void UI_WifiPopup_Connected(char *ssid);
void UI_WifiPopup_NotConnected(void);

#endif /* MAIN_UI_UI_WIFI_LIST_H_ */
