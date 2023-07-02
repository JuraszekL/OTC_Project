/*
 * display.h
 *
 *  Created on: Jun 30, 2023
 *      Author: juraszekl
 */

#ifndef MAIN_INCLUDE_DISPLAY_H_
#define MAIN_INCLUDE_DISPLAY_H_

/**************************************************************
 * Display configuration
 ***************************************************************/
#define LCD_HORIZONTAL_RES		320U
#define LCD_VERTICAL_RES		480U

#define PSRAM_DATA_ALIGNMENT   	64U

#define LCD_PIXEL_CLOCK_HZ		(20U * 1000U * 1000U)
#define LCD_CMD_BITS     		8U
#define LCD_PARAM_BITS   		8U
#define LVGL_TICK_MS			2U

	/* GPIO */
#define LCD_BACKLIGHT_GPIO		GPIO_NUM_46
#define LCD_DC_GPIO 			GPIO_NUM_45
#define LCD_WR_GPIO 			GPIO_NUM_18
#define LCD_RD_GPIO 			GPIO_NUM_48

#define LCD_D0_GPIO 			GPIO_NUM_47
#define LCD_D1_GPIO 			GPIO_NUM_21
#define LCD_D2_GPIO 			GPIO_NUM_14
#define LCD_D3_GPIO 			GPIO_NUM_13
#define LCD_D4_GPIO 			GPIO_NUM_12
#define LCD_D5_GPIO 			GPIO_NUM_11
#define LCD_D6_GPIO 			GPIO_NUM_10
#define LCD_D7_GPIO 			GPIO_NUM_9
#define LCD_D8_GPIO 			GPIO_NUM_3
#define LCD_D9_GPIO 			GPIO_NUM_8
#define LCD_D10_GPIO 			GPIO_NUM_16
#define LCD_D11_GPIO 			GPIO_NUM_15
#define LCD_D12_GPIO 			GPIO_NUM_7
#define LCD_D13_GPIO 			GPIO_NUM_6
#define LCD_D14_GPIO 			GPIO_NUM_5
#define LCD_D15_GPIO 			GPIO_NUM_4

/**************************************************************
 * Public functions
 ***************************************************************/
void Disp_PeriphInit(void);
void Display_Task(void *arg);

#endif /* MAIN_INCLUDE_DISPLAY_H_ */
