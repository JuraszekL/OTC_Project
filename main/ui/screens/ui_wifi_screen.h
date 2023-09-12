#ifndef MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_
#define MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_

#include "main.h"

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_WifiScreen_Init(void);
void UI_WifiScreen_Load(uint32_t delay);
void UI_WifiScreen_Connecting(WifiCreds_t *creds);
void UI_WifiScreen_Connected(UI_DetailedAPData_t *data);
void UI_WifiScreen_ConnectError(void);
void UI_WifiScreen_GetPass(WifiCreds_t *creds);
void UI_WifiScreen_SetApDetails(UI_DetailedAPData_t *data);
void UI_WifiScreen_AddToList(UI_BasicAPData_t *data);
void UI_WifiScreen_ClearList(void);
void UI_WifiScreen_WifiListClicked(lv_obj_t * obj);

#endif /* MAIN_UI_SCREENS_UI_WIFI_SCREEN_H_ */
