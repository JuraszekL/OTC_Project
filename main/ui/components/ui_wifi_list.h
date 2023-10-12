#ifndef MAIN_UI_COMPONENTS_UI_WIFI_LIST_H_
#define MAIN_UI_COMPONENTS_UI_WIFI_LIST_H_

#include "lvgl.h"

void UI_WifiListInit(lv_obj_t *screen);
void UI_WifiListClear(void);
void UI_WifiListAdd(bool is_protected, char *name, int rssi);

#endif /* MAIN_UI_COMPONENTS_UI_WIFI_LIST_H_ */
