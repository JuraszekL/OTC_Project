#include "main.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "lvgl.h"
#include "esp_lcd_ili9488.h"

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
#define LONG_PRESS_TIME_MS		1000U

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
 *
 *	Function prototypes
 *
 ***************************************************************/
static void lvgl_init(void);
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lcd_flush(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void lcd_LVGL_tick(void *arg);
static void lcd_get_touch_data(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void lvgl_log_cb(const char * buf);

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

SemaphoreHandle_t LVGL_MutexHandle;

/******************************************************************************************************************
 *
 * Display task
 *
 ******************************************************************************************************************/
void Display_Task(void *arg){

	BaseType_t res;

	lvgl_init();

	// mutex for LVGL operations
	LVGL_MutexHandle = xSemaphoreCreateMutex();
	assert(LVGL_MutexHandle);

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, DISPLAY_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		// perform LVGL operation
		res = xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
		if(pdTRUE == res){

	    	lv_timer_handler();
	    	xSemaphoreGive(LVGL_MutexHandle);
		}

    	vTaskDelay(1);
	}
}


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* Initialize display peripherials */
void Disp_PeriphInit(void){

	gpio_config_t lcd_rd_pin_config, lcd_backlight_pin_config;
	ledc_timer_config_t lcd_backlight_pwm_timer_config;
	ledc_channel_config_t lcd_backlight_pwm_channel_config;
	esp_lcd_i80_bus_config_t lcd_bus_config;
	esp_lcd_panel_io_i80_config_t lcd_panel_io_config;
	esp_lcd_panel_dev_config_t lcd_panel_config;

	// config GPIO
	lcd_rd_pin_config.pin_bit_mask = (1ULL << LCD_RD_GPIO);
	lcd_rd_pin_config.mode = GPIO_MODE_OUTPUT;
	lcd_rd_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	lcd_rd_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	lcd_rd_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&lcd_rd_pin_config));
	gpio_set_level(LCD_RD_GPIO, 1U);

	lcd_backlight_pin_config.pin_bit_mask = (1ULL << LCD_BACKLIGHT_GPIO);
	lcd_backlight_pin_config.mode = GPIO_MODE_OUTPUT;
	lcd_backlight_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	lcd_backlight_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	lcd_backlight_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&lcd_backlight_pin_config));

	// config LCD timer and PWM
	lcd_backlight_pwm_timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
	lcd_backlight_pwm_timer_config.duty_resolution = LEDC_TIMER_10_BIT;
	lcd_backlight_pwm_timer_config.timer_num = LEDC_TIMER_0;
	lcd_backlight_pwm_timer_config.freq_hz = 1000U;
	lcd_backlight_pwm_timer_config.clk_cfg =LEDC_AUTO_CLK;
	ESP_ERROR_CHECK(ledc_timer_config(&lcd_backlight_pwm_timer_config));

	lcd_backlight_pwm_channel_config.gpio_num = LCD_BACKLIGHT_GPIO;
	lcd_backlight_pwm_channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
	lcd_backlight_pwm_channel_config.channel = LEDC_CHANNEL_0;
	lcd_backlight_pwm_channel_config.intr_type = LEDC_INTR_DISABLE;
	lcd_backlight_pwm_channel_config.timer_sel = LEDC_TIMER_0;
	lcd_backlight_pwm_channel_config.duty = 0;
	lcd_backlight_pwm_channel_config.hpoint = 0;
	lcd_backlight_pwm_channel_config.flags.output_invert = 0;
	ESP_ERROR_CHECK(ledc_channel_config(&lcd_backlight_pwm_channel_config));

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
}

/* Set value of display backlight (0% - 100%) */
void Disp_SetBacklight(uint8_t backlight_percent){

	uint8_t a;
	uint32_t duty;

	if(100 < backlight_percent) a = 100;
	else if(5 > backlight_percent) a = 5;
	else a = backlight_percent;

	// PWM resolution is set to 10-bit, so value changes between 0 - 1023
	// this line converts percent value (0 - 100) to PWM value (0 - 1023)
	duty = (((uint32_t)1023 * (uint32_t)a) / (uint32_t)100U);

	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
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
    touch_driver.long_press_time = LONG_PRESS_TIME_MS;
    touch = lv_indev_drv_register(&touch_driver);

    // register logging function
	lv_log_register_print_cb(lvgl_log_cb);

    // initialize png converter
    lv_png_init();
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

	BaseType_t res = xSemaphoreTake(LVGL_MutexHandle, pdMS_TO_TICKS(100));
	if(pdTRUE == res){

	    lv_tick_inc(LVGL_TICK_MS);
    	xSemaphoreGive(LVGL_MutexHandle);
	}
}

/* check the queue for new data from touchpad */
static void lcd_get_touch_data(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data){

	TouchPad_Data_t tp = {0};
	int res;

	res = TouchPad_GetData(&tp);
	if(-1 == res){

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

/* logging function wrapper */
static void lvgl_log_cb(const char * buf){

	ESP_LOGW("lvgl", "%s", buf);
}
