#ifndef MAIN_UI_UI_WIFI_LIST_H_
#define MAIN_UI_UI_WIFI_LIST_H_

#include "main.h"

void UI_WifiListInit(void);
void UI_WifiListClear(void);
void UI_WifiListAdd(bool is_protected, char *name, int rssi);
void UI_WifiPopup_Connecting(WifiCreds_t *data);

#endif /* MAIN_UI_UI_WIFI_LIST_H_ */
