#include "main.h"
#include "driver/ledc.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define LCD_BACKLIGHT_GPIO		GPIO_NUM_46

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/

/**************************************************************
 *
 *	Public functions
 *
 ***************************************************************/

/* initialize peripherials */
void PWM_PeriphInit(void){

	gpio_config_t lcd_backlight_pin_config;
	ledc_timer_config_t lcd_backlight_pwm_timer_config;
	ledc_channel_config_t lcd_backlight_pwm_channel_config;

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
}

/* Set value of display backlight (0% - 100%) */
void PWM_SetBacklight(uint8_t backlight_percent){

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
 *	Private functions
 *
 ***************************************************************/
