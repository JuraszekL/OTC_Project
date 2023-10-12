#ifndef MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_
#define MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_

#include "main.h"

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_WifiPopup_MutexInit(void);
void UI_WifiPopup_Delete(void);
void UI_WifiPopup_Connecting(char *ssid);
void UI_WifiPopup_Connected(char *ssid);
void UI_WifiPopup_NotConnected(void);
void UI_WifiPopup_GetPass(WifiCreds_t *creds);
void UI_WifiPopup_PassDeleted(char *ssid);
void UI_WifiPopup_PassNotDeleted(char *ssid);

#endif /* MAIN_UI_COMPONENTS_UI_WIFI_POPUP_H_ */
