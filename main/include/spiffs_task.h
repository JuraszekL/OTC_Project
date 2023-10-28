#ifndef MAIN_INCLUDE_SPIFFS_TASK_H_
#define MAIN_INCLUDE_SPIFFS_TASK_H_

/**************************************************************
 * Public data structures
 ***************************************************************/
typedef enum {

	CONFIG_THEME = 0,
	CONFIG_BACKLIGHT,

} NVS_ConfigType_t;

/**************************************************************
 * Public functions
 ***************************************************************/
void SPIFFS_NVS_Task(void *arg);

void SPIFFS_GetPassAndConnect(char *ssid);
void SPIFFS_SavePass(WifiCreds_t *creds);
void SPIFFS_DeletePass(char *ssid);

void NVS_SetConfig(NVS_ConfigType_t type, void *arg);
void NVS_GetConfig(NVS_ConfigType_t type, void *arg);

#endif /* MAIN_INCLUDE_SPIFFS_TASK_H_ */
