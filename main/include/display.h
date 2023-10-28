/*
 * display.h
 *
 *  Created on: Jun 30, 2023
 *      Author: juraszekl
 */

#ifndef MAIN_INCLUDE_DISPLAY_H_
#define MAIN_INCLUDE_DISPLAY_H_

/**************************************************************
 * Public variable
 ***************************************************************/
extern SemaphoreHandle_t LVGL_MutexHandle;

/**************************************************************
 * Public functions
 ***************************************************************/
void Display_Task(void *arg);

void Disp_PeriphInit(void);
void Disp_SetBacklight(uint8_t backlight_percent);

#endif /* MAIN_INCLUDE_DISPLAY_H_ */
