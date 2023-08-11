#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_spiffs.h"
#include "esp_efuse.h"
#include "esp_random.h"
#include "mbedtls/aes.h"

#include "main.h"
#include "spiffs_task.h"

//static int spiffs_mount(void);

//const static char string_to_encrypt[] = "string do zaszyfrowania";
//
//static unsigned char input[128], output[128];

void SPIFFS_Task(void *arg){

//	spiffs_mount();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SPIFFS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	while(1){

		vTaskSuspend(NULL);
	}
}

//static int spiffs_mount(void){
//
//	FILE *f = 0;
//	size_t org_len, new_len, a;
//	uint8_t y, z;
//
//	mbedtls_aes_context aes_ctx;
//	unsigned char aes_key[32], iv[16], iv2[16];
//
//	const esp_efuse_desc_t **aes_key_efuse;
//
//    esp_vfs_spiffs_conf_t conf = {
//      .base_path = "/spiffs",
//      .partition_label = "spiffs",
//      .max_files = 3,
//      .format_if_mount_failed = true,
//    };
//
//    esp_err_t ret = esp_vfs_spiffs_register(&conf);
//
//    if (ret != ESP_OK) {
//        if (ret == ESP_FAIL) {
//            ESP_LOGE("", "Failed to mount or format filesystem");
//        } else if (ret == ESP_ERR_NOT_FOUND) {
//            ESP_LOGE("", "Failed to find SPIFFS partition");
//        } else {
//            ESP_LOGE("", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
//        }
//        return -1;
//    }
//
//    size_t total = 0, used = 0;
//    ret = esp_spiffs_info(conf.partition_label, &total, &used);
//    if (ret != ESP_OK) {
//        ESP_LOGE("", "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
//        esp_spiffs_format(conf.partition_label);
//        return -1;
//    } else {
//        ESP_LOGI("", "Partition size: total: %d, used: %d", total, used);
//    }
//
//    aes_key_efuse = esp_efuse_get_key(EFUSE_BLK_KEY0);
//    ret = esp_efuse_read_field_blob(aes_key_efuse, aes_key, (sizeof(aes_key) * 8));
//    if(ESP_OK != ret){
//
//    	ESP_LOGE("", "Failed to read efuse");
//    	return -1;
//    }
//
//    esp_fill_random(iv, 16U);
//	memcpy(iv2, iv, 16);
//    mbedtls_aes_init(&aes_ctx);
//    mbedtls_aes_setkey_enc(&aes_ctx, aes_key, 256);
//    mbedtls_aes_setkey_dec(&aes_ctx, aes_key, 256);
//
//    org_len = strlen(string_to_encrypt);	// len = 23
//    if(128 == org_len) return -1;
//    new_len = org_len;
//
//    z = org_len % 16U;	// how many characters are left and need to be padded	// z = 7
//    memcpy(input, string_to_encrypt, org_len);
//
//    // if there are characters to be padded
//    if(0 != z){
//
//    	y = 16U - z;		// y = 9
//    	while(0 != y){
//
//    		input[new_len] = '0';
//    		new_len++;
//    		y--;
//    	}
//    }
//
//    z = new_len % 16U;
//    if(0 != z){
//
//    	ESP_LOGE("", "algorithm is fucked up a lot...");
//    	return -1;
//    }
//    else{
//
//    	ESP_LOGI("", "org_len = %d, new_len = %d", org_len, new_len);
//    }
//
//    if(0 != mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_ENCRYPT, new_len, iv, input, output )){
//
//    	ESP_LOGE("", "crypt has failed");
//    	return -1;
//    }
//
//    new_len = strnlen((char *)output, 128);
//    ESP_LOGI("", "new_len = %d", new_len);
//
//
//    f = fopen("/spiffs/wifi", "w+");
//    if(0 == f){
//
//    	ESP_LOGE("", "Failed to create file");
//    	return -1;
//    }
//
//    a = fwrite(output, sizeof(char), new_len + 1, f);
//
//    ESP_LOGI("", "a = %d", a);
//
//    rewind(f);
//    memset(input, 0, sizeof(input));
//    memset(output, 0, sizeof(output));
//
//    a = fread(input, sizeof(char), new_len + 1, f);
//
//    if(0 != mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_DECRYPT, new_len, iv2, input, output )){
//
//    	ESP_LOGE("", "crypt has failed");
//    	return -1;
//    }
//
//    output[org_len] = 0;
//    ESP_LOGI("", "%s", output);
//
//    fclose(f);
//
//    mbedtls_aes_free(&aes_ctx);
//
//    esp_vfs_spiffs_unregister(conf.partition_label);
//
//	return 0;
//}
