/**************************************************************
 *	include
 ***************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"

#include "driver/gpio.h"

#include "esp_lcd_ili9488.h"
#include "lvgl.h"
#include "ft6x36.h"

#include "display.h"
#include "touchpad.h"
#include "ui.h"
#include "animations.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void lvgl_init(void);
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lcd_flush(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void lcd_LVGL_tick(void *arg);
static void lcd_get_touch_data(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
	// LCD peripherials init
static esp_lcd_i80_bus_handle_t lcd_bus_handle;
static esp_lcd_panel_io_handle_t lcd_panel_io_handle;
static esp_lcd_panel_handle_t lcd_panel_handle;

	// LVGL lcd init
static lv_disp_draw_buf_t lcd_buff;
static lv_disp_drv_t lcd_driver;
static lv_disp_t *disp;

	// LVGL timer
static esp_timer_handle_t lvgl_tick_timer;

	// LVGL touch init
static lv_indev_drv_t touch_driver;
static lv_indev_t *touch;

	// LVGL buffers
static EXT_RAM_BSS_ATTR lv_color_t buf1[LCD_HORIZONTAL_RES * LCD_VERTICAL_RES * sizeof(lv_color_t)];
static EXT_RAM_BSS_ATTR lv_color_t buf2[LCD_HORIZONTAL_RES * LCD_VERTICAL_RES * sizeof(lv_color_t)];


/******************************************************************************************************************
 *
 * Display task
 *
 ******************************************************************************************************************/
void Display_Task(void *arg){

//	lv_style_t Style_Invisible;

	lvgl_init();
//
//	lv_style_init(&Style_Invisible);
//	lv_style_set_bg_opa(&Style_Invisible, 0);


//	ui_init();
	ui_InitScreen_screen_init();
	lv_obj_set_style_bg_opa(ui_InitScreenPanel, 0, 0);
	lv_obj_set_style_text_opa(ui_OnlineTableClockLabel, 0, 0);
	lv_obj_set_style_text_opa(ui_ByJuraszekLLabel, 0, 0);
	lv_obj_set_style_shadow_opa(ui_InitScreenPanel, 0, 0);
	lv_disp_load_scr(ui_InitScreen);


	Anm_InitScr1200msOpa();

	while(1){

    	lv_timer_handler();
    	vTaskDelay(pdMS_TO_TICKS(10));
	}
}


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Initialize display peripherials */
void Disp_PeriphInit(void){

	gpio_config_t lcd_backlight_pin_config, lcd_rd_pin_config;
	esp_lcd_i80_bus_config_t lcd_bus_config;
	esp_lcd_panel_io_i80_config_t lcd_panel_io_config;
	esp_lcd_panel_dev_config_t lcd_panel_config;

	// config GPIO
	lcd_backlight_pin_config.pin_bit_mask = (1ULL << LCD_BACKLIGHT_GPIO);
	lcd_backlight_pin_config.mode = GPIO_MODE_OUTPUT;
	lcd_backlight_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	lcd_backlight_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	lcd_backlight_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&lcd_backlight_pin_config));

	lcd_rd_pin_config.pin_bit_mask = (1ULL << LCD_RD_GPIO);
	lcd_rd_pin_config.mode = GPIO_MODE_OUTPUT;
	lcd_rd_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	lcd_rd_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	lcd_rd_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&lcd_rd_pin_config));
	gpio_set_level(LCD_RD_GPIO, 1U);

	// config lcd I/O and I8080 bus
	lcd_bus_config.dc_gpio_num = LCD_DC_GPIO;
	lcd_bus_config.wr_gpio_num = LCD_WR_GPIO;
	lcd_bus_config.clk_src = LCD_CLK_SRC_DEFAULT;
	lcd_bus_config.data_gpio_nums[0] = LCD_D0_GPIO;
	lcd_bus_config.data_gpio_nums[1] = LCD_D1_GPIO;
	lcd_bus_config.data_gpio_nums[2] = LCD_D2_GPIO;
	lcd_bus_config.data_gpio_nums[3] = LCD_D3_GPIO;
	lcd_bus_config.data_gpio_nums[4] = LCD_D4_GPIO;
	lcd_bus_config.data_gpio_nums[5] = LCD_D5_GPIO;
	lcd_bus_config.data_gpio_nums[6] = LCD_D6_GPIO;
	lcd_bus_config.data_gpio_nums[7] = LCD_D7_GPIO;
	lcd_bus_config.data_gpio_nums[8] = LCD_D8_GPIO;
	lcd_bus_config.data_gpio_nums[9] = LCD_D9_GPIO;
	lcd_bus_config.data_gpio_nums[10] = LCD_D10_GPIO;
	lcd_bus_config.data_gpio_nums[11] = LCD_D11_GPIO;
	lcd_bus_config.data_gpio_nums[12] = LCD_D12_GPIO;
	lcd_bus_config.data_gpio_nums[13] = LCD_D13_GPIO;
	lcd_bus_config.data_gpio_nums[14] = LCD_D14_GPIO;
	lcd_bus_config.data_gpio_nums[15] = LCD_D15_GPIO;
	lcd_bus_config.bus_width = 16U;
	lcd_bus_config.max_transfer_bytes = LCD_HORIZONTAL_RES * LCD_VERTICAL_RES * sizeof(uint16_t);
	lcd_bus_config.psram_trans_align = PSRAM_DATA_ALIGNMENT,
	lcd_bus_config.sram_trans_align = 4;
	ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&lcd_bus_config, &lcd_bus_handle));

	lcd_panel_io_config.cs_gpio_num = -1;
	lcd_panel_io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ;
	lcd_panel_io_config.trans_queue_depth = 10;
	lcd_panel_io_config.on_color_trans_done = lcd_flush_ready;
	lcd_panel_io_config.user_ctx = &lcd_driver;
	lcd_panel_io_config.lcd_cmd_bits = LCD_CMD_BITS;
	lcd_panel_io_config.lcd_param_bits = LCD_PARAM_BITS;
	lcd_panel_io_config.dc_levels.dc_idle_level = 0;
	lcd_panel_io_config.dc_levels.dc_cmd_level = 0;
	lcd_panel_io_config.dc_levels.dc_dummy_level = 0;
	lcd_panel_io_config.dc_levels.dc_data_level = 1;
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(lcd_bus_handle, &lcd_panel_io_config, &lcd_panel_io_handle));

	lcd_panel_config.reset_gpio_num = -1;
	lcd_panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;
	lcd_panel_config.bits_per_pixel = 16U;
	lcd_panel_config.flags.reset_active_high = 1;
	lcd_panel_config.vendor_config = 0;
	ESP_ERROR_CHECK(esp_lcd_new_panel_ili9488(lcd_panel_io_handle, &lcd_panel_config,
			LCD_HORIZONTAL_RES * 20 * sizeof(lv_color_t), &lcd_panel_handle));

	// reset and initialize display
    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);

    // backlight ON
	gpio_set_level(LCD_BACKLIGHT_GPIO, 1U);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* Initialize LVGL and connect lcd driver */
static void lvgl_init(void){

	esp_timer_create_args_t lcd_LVGL_timer_args;

	// LVGL init
	lv_init();
	lv_disp_draw_buf_init(&lcd_buff, buf1, buf2, (LCD_HORIZONTAL_RES * LCD_VERTICAL_RES));
    lv_disp_drv_init(&lcd_driver);
    lcd_driver.hor_res = LCD_HORIZONTAL_RES;
    lcd_driver.ver_res = LCD_VERTICAL_RES;
    lcd_driver.flush_cb = lcd_flush;
    lcd_driver.draw_buf = &lcd_buff;
    lcd_driver.user_data = lcd_panel_handle;
    disp = lv_disp_drv_register(&lcd_driver);

    // timer for LVGL ticks
    lcd_LVGL_timer_args.callback = lcd_LVGL_tick;
    ESP_ERROR_CHECK(esp_timer_create(&lcd_LVGL_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_MS * 1000));

    // touch driver for LVGL
    lv_indev_drv_init(&touch_driver);
    touch_driver.type = LV_INDEV_TYPE_POINTER;
    touch_driver.read_cb = lcd_get_touch_data;
    touch_driver.disp = disp;
    touch = lv_indev_drv_register(&touch_driver);
}

/* Inform LVGL that all data were sent to display */
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx){

    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
	return 0;
}

/* Send data to display */
static void lcd_flush(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p){

    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) disp_drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);
}

/* Tell LVGL how many milliseconds has elapsed */
static void lcd_LVGL_tick(void *arg){

    lv_tick_inc(LVGL_TICK_MS);
}

/* check the queue for new data from touchpad */
static void lcd_get_touch_data(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data){

	TouchPad_Data_t tp = {0};
	esp_err_t res;

	res = xQueueReceive(TouchPad_QueueHandle, &tp, 0);
	if((pdFALSE == res) || (lifted_up == tp.evt) || (no_event == tp.evt)){

		data->state = LV_INDEV_STATE_RELEASED;
		return;
	}
	else {

		data->state = LV_INDEV_STATE_PRESSED;
		data->point.x = tp.x;
		data->point.y = tp.y;
		return;
	}
}
