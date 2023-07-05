#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "ft6x36.h"

#include "main.h"
#include "touchpad.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void touchpad_init(void);
static int8_t ft6x36_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *driver);
static int8_t ft6x36_write_platform_spec(uint8_t reg_addr, uint8_t value, void *driver);
static void touchpad_interrupt(void *arg);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static i2c_port_t TouchPad_I2CPortNr;
static FT6X36_Driver_t TouchPad_Driver;
static TaskHandle_t TouchPad_TaskHandle;

QueueHandle_t TouchPad_QueueHandle;

/******************************************************************************************************************
 *
 * TouchPad task
 *
 ******************************************************************************************************************/
void TouchPad_Task(void *arg){

	uint32_t notification_value;
	TouchPad_Data_t data;

	touchpad_init();

//	xEventGroupSync(AppStartSyncEvt, TOUCHPAD_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		notification_value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(0 != notification_value){

			if(FT6X36_GetTouchPointEXY(&data.evt, &data.x, &data.y) == FT6X36_OK){

				xQueueOverwrite(TouchPad_QueueHandle, &data);
			}
		}
	}
}

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Initialize touchpad peripherials */
void TouchPad_PeriphInit(void){

	i2c_config_t i2c0_conf;
	gpio_config_t tp_int_gpio_conf;

	/* set I2C bus pins and parameters */
	i2c0_conf.mode = I2C_MODE_MASTER;
	i2c0_conf.scl_io_num = I2C_SCL_GPIO;
	i2c0_conf.sda_io_num = I2C_SDA_GPIO;
	i2c0_conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	i2c0_conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	i2c0_conf.master.clk_speed = I2C_CLK_FREQ;
	i2c0_conf.clk_flags = 0;

	TouchPad_I2CPortNr = I2C_PORT_NR;

	ESP_ERROR_CHECK(i2c_param_config(TouchPad_I2CPortNr, &i2c0_conf));
	ESP_ERROR_CHECK(i2c_driver_install(TouchPad_I2CPortNr, i2c0_conf.mode, 0, 0, 0));

	tp_int_gpio_conf.pin_bit_mask = (1ULL << TOUCHPAD_INT_GPIO);
	tp_int_gpio_conf.mode = GPIO_MODE_INPUT;
	tp_int_gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	tp_int_gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	tp_int_gpio_conf.intr_type = GPIO_INTR_NEGEDGE;

	ESP_ERROR_CHECK(gpio_config(&tp_int_gpio_conf));
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* Initialize FT6X36 controller */
static void touchpad_init(void){

	TouchPad_TaskHandle = xTaskGetCurrentTaskHandle();

	TouchPad_Driver.read = ft6x36_read_platform_spec;
	TouchPad_Driver.write = ft6x36_write_platform_spec;
	ESP_ERROR_CHECK(FT6X36_Init(&TouchPad_Driver));

	// configure interrupt from touch controller
	gpio_install_isr_service(0);
	gpio_isr_handler_add(TOUCHPAD_INT_GPIO, touchpad_interrupt, NULL);

	// initialize queue with touchpad data
	TouchPad_QueueHandle = xQueueCreate(1, sizeof(TouchPad_Data_t));
}

/* function to read data from touch controller */
static int8_t ft6x36_read_platform_spec(uint8_t reg_addr, uint8_t *rxbuff, uint8_t rxlen, void *env_spec){

	/* check parameter */
	if(NULL == rxbuff) return -1;

	/* prepare local variables */
	esp_err_t res;
	i2c_cmd_handle_t read_cmd;

	/* create new command list */
	read_cmd = i2c_cmd_link_create();

	/* fill the list with proper commands */
	i2c_master_start(read_cmd);
	i2c_master_write_byte(read_cmd, (FT6X36_I2C_ADDR << 1U), 1);
	i2c_master_write_byte(read_cmd, reg_addr, 1);

	i2c_master_start(read_cmd);
	i2c_master_write_byte(read_cmd, ((FT6X36_I2C_ADDR << 1U) + 1), 1);
	i2c_master_read(read_cmd, rxbuff, rxlen, I2C_MASTER_LAST_NACK);
	i2c_master_stop(read_cmd);

	/* start i2c transmission */
	res = i2c_master_cmd_begin(TouchPad_I2CPortNr, read_cmd, pdMS_TO_TICKS(500));

	/* delete command list */
	i2c_cmd_link_delete(read_cmd);


	if(ESP_OK != res) return -1;
	return 0;
}

/* function to write data to touch controller */
static int8_t ft6x36_write_platform_spec(uint8_t reg_addr, uint8_t value, void *env_spec){

	/* prepare local variables */
	esp_err_t res;
	i2c_cmd_handle_t write_cmd;

	/* create new command list */
	write_cmd = i2c_cmd_link_create();

	/* fill the list with proper commands */
	i2c_master_start(write_cmd);
	i2c_master_write_byte(write_cmd, (FT6X36_I2C_ADDR << 1U), 1);
	i2c_master_write_byte(write_cmd, reg_addr, 1);
	i2c_master_write_byte(write_cmd, value, 1);
	i2c_master_stop(write_cmd);

	/* start i2c transmission */
	res = i2c_master_cmd_begin(TouchPad_I2CPortNr, write_cmd, pdMS_TO_TICKS(500));

	/* delete command list */
	i2c_cmd_link_delete(write_cmd);

	if(ESP_OK != res) return -1;
	return 0;
}

static void IRAM_ATTR touchpad_interrupt(void *arg){

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(TouchPad_TaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
