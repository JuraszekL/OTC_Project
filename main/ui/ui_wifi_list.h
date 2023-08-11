#ifndef MAIN_UI_UI_WIFI_LIST_H_
#define MAIN_UI_UI_WIFI_LIST_H_

#include "lvgl.h"
#include "ui.h"
#include "ui_task.h"

void UI_WifiListInit(void);
void UI_WifiListClear(void);
void UI_WifiListAdd(bool is_protected, char *name, int rssi);

#endif /* MAIN_UI_UI_WIFI_LIST_H_ */
