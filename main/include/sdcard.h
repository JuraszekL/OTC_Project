#ifndef MAIN_INCLUDE_SDCARD_H_
#define MAIN_INCLUDE_SDCARD_H_


/**************************************************************
 * Public mounting point path
 ***************************************************************/
extern const char mount_point[];

/**************************************************************
 * Public functions
 ***************************************************************/
void SDCard_Task(void *arg);

#endif /* MAIN_INCLUDE_SDCARD_H_ */
