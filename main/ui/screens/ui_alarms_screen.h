#ifndef MAIN_UI_SCREENS_UI_ALARMS_SCREEN_H_
#define MAIN_UI_SCREENS_UI_ALARMS_SCREEN_H_

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_AlarmsScreen_Init(void);
void UI_AlarmsScreen_Load(void);
void UI_AlarmsScreen_EditAlarm(uint8_t idx);
void UI_AlarmScreen_RefreshPanel(uint8_t idx);

#endif /* MAIN_UI_SCREENS_UI_ALARMS_SCREEN_H_ */
