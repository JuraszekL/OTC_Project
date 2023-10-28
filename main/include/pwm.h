#ifndef MAIN_INCLUDE_PWM_H_
#define MAIN_INCLUDE_PWM_H_

/**************************************************************
 * Public functions
 ***************************************************************/
void PWM_Task(void *arg);

void PWM_PeriphInit(void);
void PWM_SetBacklight(uint8_t backlight_percent);
void Buzzer_Beep1(void);

#endif /* MAIN_INCLUDE_PWM_H_ */
