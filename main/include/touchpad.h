#ifndef MAIN_INCLUDE_TOUCHPAD_H_
#define MAIN_INCLUDE_TOUCHPAD_H_

/**************************************************************
 * Typedefs
 ***************************************************************/
typedef struct {

	uint8_t evt;
	uint16_t x;
	uint16_t y;

} TouchPad_Data_t;

/**************************************************************
 * Public functions
 ***************************************************************/
void TouchPad_Task(void *arg);

void TouchPad_PeriphInit(void);
int TouchPad_GetData(TouchPad_Data_t *data);

#endif /* MAIN_INCLUDE_TOUCHPAD_H_ */
