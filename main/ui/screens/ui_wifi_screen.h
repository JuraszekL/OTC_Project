#ifndef MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_
#define MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_

#include "main.h"

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_WifiScreen_Init(void);
void UI_WifiScreen_Load(uint32_t delay);
void UI_WifiScreen_SetApDetails(UI_DetailedAPData_t *data);

void UI_WifiScreen_PopupConnecting(WifiCreds_t *creds);
void UI_WifiScreen_PopupConnected(UI_DetailedAPData_t *data);
void UI_WifiScreen_PopupConnectError(void);
void UI_WifiScreen_PopupGetPass(WifiCreds_t *creds);
void UI_WifiScreen_PopupDelete(void);

void UI_WifiScreen_WifiListClear(void);
void UI_WifiScreen_WifiListAdd(UI_BasicAPData_t *data);
void UI_WifiScreen_WifiListClicked(lv_obj_t * obj);

#endif /* MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_ */
