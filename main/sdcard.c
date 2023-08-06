#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "main.h"
#include "sdcard.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define SD_MOUNT_POINT 			"/sdcard"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static esp_err_t initialize_sdcard(void);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
const char mount_point[] = SD_MOUNT_POINT;

static sdmmc_card_t *card;
static const sdmmc_host_t host = SDSPI_HOST_DEFAULT();

/******************************************************************************************************************
 *
 * SDCARD task
 *
 ******************************************************************************************************************/
void SDCard_Task(void *arg){

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SDCARD_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		if(ESP_OK == initialize_sdcard()){

			ESP_LOGI("", "card initialized!");
			vTaskDelete(NULL);
		}
		else{

			ESP_LOGE("", "retrying card init");
			vTaskDelay(100);
		}
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* initialize sd card */
static esp_err_t initialize_sdcard(void){

	 esp_err_t res;

	 esp_vfs_fat_sdmmc_mount_config_t mount_config = {

			 .format_if_mount_failed = SDCARD_FORMAT_IF_FAILED,
			 .max_files = 5,
			 .allocation_unit_size = 0,
	 };

	 spi_bus_config_t bus_cfg = {

			 .mosi_io_num = SDCARD_MOSI_GPIO,
			 .miso_io_num = SDCARD_MISO_GPIO,
			 .sclk_io_num = SDCARD_SCLK_GPIO,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.max_transfer_sz = 10000U,
	 };

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = SDCARD_CS_GPIO;
	slot_config.host_id = host.slot;

	 res = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
	 if(ESP_OK != res) goto cleanup;

	 res = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	 if(ESP_OK != res) goto cleanup;

	 return res;

	 cleanup:
	 	 if(card) esp_vfs_fat_sdcard_unmount(mount_point, card);
	 	 spi_bus_free(host.slot);
	 	 return res;
}
