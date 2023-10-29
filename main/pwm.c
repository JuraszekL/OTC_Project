#include "main.h"
#include "driver/ledc.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define PWM_SPEED_MODE				LEDC_LOW_SPEED_MODE

// backlight
#define LCD_BACKLIGHT_GPIO			GPIO_NUM_46
#define LCD_BACKLIGHT_TIMER_NR		LEDC_TIMER_0
#define LCD_BACKLIGHT_PWM_FREQ		1000U
#define LCD_BACKLIGHT_CHANNEL_NR	LEDC_CHANNEL_0

// buzzer
#define BUZZER_GPIO					GPIO_NUM_20
#define BUZZER_TIMER_NR				LEDC_TIMER_1
#define BUZZER_PWM_ON				127U
#define BUZZER_PWM_OFF				0U
#define BUZZER_CHANNEL_NR			LEDC_CHANNEL_1

// basic notes frequencies
#define C_NOTE_FREQ					262U
#define D_NOTE_FREQ					294U
#define E_NOTE_FREQ					330U
#define F_NOTE_FREQ					349U
#define G_NOTE_FREQ					392U
#define A_NOTE_FREQ					440U
#define B_NOTE_FREQ					494U

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef enum {

	buzzer_beep_1 = 0,
	buzzer_beep_2,

} pwm_routine_type_t;

typedef void (*pwm_routine)(void *arg);

struct pwm_queue_data {

	pwm_routine_type_t type;
	void *arg;
};

struct buzzer_sequence_data {

	uint16_t freq;
	uint16_t time_ms;
	uint8_t buzzer_duty;
};

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void pwm_routine_request(pwm_routine_type_t type, void *arg);

static void pwm_beep1_routine(void *arg);
static void pwm_beep2_routine(void *arg);

static void buzzer_stop(void);
static void buzzer_play_sequence(const struct buzzer_sequence_data *sequence);
static void buzzer_set_freq(uint16_t freq);
static void buzzer_set_duty(uint8_t duty);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static QueueHandle_t pwm_queue_handle;
static TaskHandle_t pwm_task_handle;

static const pwm_routine pwm_routines_tab[] = {

		[buzzer_beep_1] = pwm_beep1_routine,
		[buzzer_beep_2] = pwm_beep2_routine,
};

// example sound played by buzzer
static const struct buzzer_sequence_data ok_sound[] = {

		{
				.freq = C_NOTE_FREQ,
				.time_ms = 200,
				.buzzer_duty = BUZZER_PWM_ON,
		},
		{
				.freq = E_NOTE_FREQ,
				.time_ms = 200,
				.buzzer_duty = BUZZER_PWM_ON,
		},
		{
				.freq = C_NOTE_FREQ,
				.time_ms = 200,
				.buzzer_duty = BUZZER_PWM_ON,
		},
		{

				.freq = 0,
		}
};

/******************************************************************************************************************
 *
 * PWM task
 *
 ******************************************************************************************************************/
void PWM_Task(void *arg){

	BaseType_t ret;
	struct pwm_queue_data data;

	pwm_queue_handle = xQueueCreate(1U, sizeof(struct pwm_queue_data));
	assert(pwm_queue_handle);

	pwm_task_handle = xTaskGetCurrentTaskHandle();

	xEventGroupSync(AppStartSyncEvt, PWM_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		ret = xQueueReceive(pwm_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			pwm_routines_tab[data.type](data.arg);
		}
	}
}

/**************************************************************
 *
 *	Public functions
 *
 ***************************************************************/

/* initialize peripherials */
void PWM_PeriphInit(void){

	gpio_config_t lcd_backlight_pin_config, buzzer_pin_config;
	ledc_timer_config_t lcd_backlight_pwm_timer_config, buzzer_pwm_timer_config;
	ledc_channel_config_t lcd_backlight_pwm_channel_config, buzzer_pwm_channel_config;

	// GPIO configs
	lcd_backlight_pin_config.pin_bit_mask = (1ULL << LCD_BACKLIGHT_GPIO);
	lcd_backlight_pin_config.mode = GPIO_MODE_OUTPUT;
	lcd_backlight_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	lcd_backlight_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	lcd_backlight_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&lcd_backlight_pin_config));

	buzzer_pin_config.pin_bit_mask = (1ULL << BUZZER_GPIO);
	buzzer_pin_config.mode = GPIO_MODE_OUTPUT;
	buzzer_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	buzzer_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	buzzer_pin_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&buzzer_pin_config));

	// timers and PWM's configs
	lcd_backlight_pwm_timer_config.speed_mode = PWM_SPEED_MODE;
	lcd_backlight_pwm_timer_config.duty_resolution = LEDC_TIMER_10_BIT;
	lcd_backlight_pwm_timer_config.timer_num = LCD_BACKLIGHT_TIMER_NR;
	lcd_backlight_pwm_timer_config.freq_hz = LCD_BACKLIGHT_PWM_FREQ;
	lcd_backlight_pwm_timer_config.clk_cfg =LEDC_AUTO_CLK;
	ESP_ERROR_CHECK(ledc_timer_config(&lcd_backlight_pwm_timer_config));

	buzzer_pwm_timer_config.speed_mode = PWM_SPEED_MODE;
	buzzer_pwm_timer_config.duty_resolution = LEDC_TIMER_8_BIT;
	buzzer_pwm_timer_config.timer_num = BUZZER_TIMER_NR;
	buzzer_pwm_timer_config.freq_hz = C_NOTE_FREQ;
	buzzer_pwm_timer_config.clk_cfg =LEDC_AUTO_CLK;
	ESP_ERROR_CHECK(ledc_timer_config(&buzzer_pwm_timer_config));

	lcd_backlight_pwm_channel_config.gpio_num = LCD_BACKLIGHT_GPIO;
	lcd_backlight_pwm_channel_config.speed_mode = PWM_SPEED_MODE;
	lcd_backlight_pwm_channel_config.channel = LCD_BACKLIGHT_CHANNEL_NR;
	lcd_backlight_pwm_channel_config.intr_type = LEDC_INTR_DISABLE;
	lcd_backlight_pwm_channel_config.timer_sel = LCD_BACKLIGHT_TIMER_NR;
	lcd_backlight_pwm_channel_config.duty = 0;
	lcd_backlight_pwm_channel_config.hpoint = 0;
	lcd_backlight_pwm_channel_config.flags.output_invert = 0;
	ESP_ERROR_CHECK(ledc_channel_config(&lcd_backlight_pwm_channel_config));

	buzzer_pwm_channel_config.gpio_num = BUZZER_GPIO;
	buzzer_pwm_channel_config.speed_mode = PWM_SPEED_MODE;
	buzzer_pwm_channel_config.channel = BUZZER_CHANNEL_NR;
	buzzer_pwm_channel_config.intr_type = LEDC_INTR_DISABLE;
	buzzer_pwm_channel_config.timer_sel = BUZZER_TIMER_NR;
	buzzer_pwm_channel_config.duty = BUZZER_PWM_OFF;
	buzzer_pwm_channel_config.hpoint = 0;
	buzzer_pwm_channel_config.flags.output_invert = 0;
	ESP_ERROR_CHECK(ledc_channel_config(&buzzer_pwm_channel_config));
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

	ledc_set_duty(PWM_SPEED_MODE, LCD_BACKLIGHT_CHANNEL_NR, duty);
	ledc_update_duty(PWM_SPEED_MODE, LCD_BACKLIGHT_CHANNEL_NR);
}

/* example of buzzer sound */
void Buzzer_Beep1(void){

	buzzer_stop();
	pwm_routine_request(buzzer_beep_1, NULL);
}

/**************************************************************
 *
 *	Private functions
 *
 ***************************************************************/

/* send routine to be performed */
static void pwm_routine_request(pwm_routine_type_t type, void *arg){

	struct pwm_queue_data data;

	data.type = type;
	data.arg = arg;

	xQueueOverwrite(pwm_queue_handle, &data);
}

/* example sound */
static void pwm_beep1_routine(void *arg){

	buzzer_play_sequence(ok_sound);
}

static void pwm_beep2_routine(void *arg){


}


/*///////////////////////////////////////////////////
 *
 * Helpers
 *
 * */////////////////////////////////////////////////

/* send notification to pwm task to stop playing current sequence */
static void buzzer_stop(void){

	xTaskNotifyGive(pwm_task_handle);
}

/* play the sequence */
static void buzzer_play_sequence(const struct buzzer_sequence_data *seq){

	uint8_t a;
	uint32_t notification_value;

	// clear any pending notification
	ulTaskNotifyTake(pdTRUE, 0);

	// set every value to buzzer step by step
	for(a = 0; (0 != seq[a].freq); a++){

		buzzer_set_freq(seq[a].freq);
		buzzer_set_duty(seq[a].buzzer_duty);
		notification_value = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(seq[a].time_ms));	// wait untill time ends
		if(0 != notification_value){													// or notification comes

			buzzer_set_duty(BUZZER_PWM_OFF);
			return;
		}
	}

	buzzer_set_duty(BUZZER_PWM_OFF);
}

/* set frequency */
static void buzzer_set_freq(uint16_t freq){

	ledc_set_freq(PWM_SPEED_MODE, BUZZER_TIMER_NR, freq);
}

/* set duty cycle (0 = buzzer off, 127 = buzzer on) */
static void buzzer_set_duty(uint8_t duty){

	ledc_set_duty(PWM_SPEED_MODE, BUZZER_CHANNEL_NR, duty);
	ledc_update_duty(PWM_SPEED_MODE, BUZZER_CHANNEL_NR);
}
