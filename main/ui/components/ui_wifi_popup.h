#ifndef MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_
#define MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_

#include "main.h"

void UI_WifiPopup_MutexInit(void);
void UI_WifiPopup_Delete(void);
void UI_WifiPopup_Connecting(char *ssid);
void UI_WifiPopup_Connected(char *ssid);
void UI_WifiPopup_NotConnected(void);
void UI_WifiPopup_GetPass(WifiCreds_t *creds);

#endif /* MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_ */
