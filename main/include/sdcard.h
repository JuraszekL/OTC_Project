#ifndef MAIN_INCLUDE_SDCARD_H_
#define MAIN_INCLUDE_SDCARD_H_

/**************************************************************
 * SDCARD config
 ***************************************************************/
#define SDCARD_FORMAT_IF_FAILED			1

#define SDCARD_MOSI_GPIO				GPIO_NUM_2
#define SDCARD_MISO_GPIO				GPIO_NUM_41
#define SDCARD_SCLK_GPIO				GPIO_NUM_42
#define SDCARD_CS_GPIO					GPIO_NUM_1

/**************************************************************
 * Public mounting point path
 ***************************************************************/
extern const char mount_point[];

/**************************************************************
 * Public functions
 ***************************************************************/
void SDCard_Task(void *arg);


#endif /* MAIN_INCLUDE_SDCARD_H_ */
