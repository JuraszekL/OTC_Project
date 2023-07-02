#ifndef MAIN_INCLUDE_TOUCHPAD_H_
#define MAIN_INCLUDE_TOUCHPAD_H_

#include "freertos/queue.h"

/**************************************************************
 * I2C configuration
 ***************************************************************/
#define I2C_PORT_NR			I2C_NUM_0
#define I2C_SCL_GPIO		GPIO_NUM_39
#define I2C_SDA_GPIO		GPIO_NUM_38
#define I2C_CLK_FREQ		100000U

/**************************************************************
 * Touchpad interrupt configuration
 ***************************************************************/
#define TOUCHPAD_INT_GPIO	GPIO_NUM_0

/**************************************************************
 * Typedefs
 ***************************************************************/
typedef struct {

	uint8_t evt;
	uint16_t x;
	uint16_t y;

} TouchPad_Data_t;

/**************************************************************
 * Public variables
 ***************************************************************/
extern QueueHandle_t TouchPad_QueueHandle;

/**************************************************************
 * Public functions
 ***************************************************************/
void TouchPad_PeriphInit(void);
void TouchPad_Task(void *arg);

#endif /* MAIN_INCLUDE_TOUCHPAD_H_ */
