#ifndef MAIN_UI_UI_MAIN_SCREEN_H_
#define MAIN_UI_UI_MAIN_SCREEN_H_

#include "main.h"
#include <sys/time.h>

/**************************************************************
 * Type of icon to be set
 ***************************************************************/
typedef enum {wifi_icon = 0, sync_icon = 3} UI_TopIconType_t;

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_MainScreen_Init(void);
void UI_MainScreen_Load(uint32_t delay);
void UI_MainScreen_UpdateWeatherIcon(char *icon_path);
void UI_MainScreen_UpdateWeatherData(int temp, int press, int hum);
void UI_MainScreen_SetTopIcon(UI_TopIconType_t type, char icon);
void UI_MainScreen_UpdateTimeAndDate(struct tm *changed_time);

#endif /* MAIN_UI_UI_MAIN_SCREEN_H_ */
