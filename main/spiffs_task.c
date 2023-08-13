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
#include "errno.h"

#include "cJSON.h"

#include "main.h"
#include "ui_task.h"
#include "spiffs_task.h"

//#define FORMAT_SPIFFS
//#define ZJEB_DANE

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define WIFI_PASS_FILE_EXIST_BIT			(1U << 0)
//#define WIFI_PASS_FILE_EMPTY_BIT			(1U << 1)

#define SPIFFS_MOUNT_PATH 					"/spiffs"
#define SPIFFS_PART_LABEL 					"spiffs"
#define SPIFFS_WIFI_PASS_FILENAME			"/wifi_pass"
#define SPIFFS_WIFI_PASS_BCKUP_FILENAME		"/wifi_pass_backup"

#define JSON_IV_LABEL						"iv"
#define JSON_PASS_LABEL						"pass"
#define JSON_ORG_LEN_LABEL					"org_len"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/
typedef enum {

	spiffs_check_wifi_pass_file = 0,
	spiffs_check_record,
	spiffs_add_record,

} spiffs_routine_t;

typedef void(*spiffs_routine)(void *arg);

struct spiffs_queue_data {

	spiffs_routine_t type;
	void *arg;

};

typedef struct {

	char *ssid;
	char *iv;
	char *pass;
	int org_len;

} spiffs_wifi_record_t;

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void spiffs_mount(void);
static void spiffs_routine_request(spiffs_routine_t type, void *arg, bool important);

static void spiffs_check_wifi_pass_file_routine(void *arg);
static void spiffs_check_record_routine(void *arg);
static void spiffs_add_record_routine(void *arg);

static int spiffs_restore_wifi_pass_backup_file(void);
static void spiffs_add_wifi_record(spiffs_wifi_record_t *data);
static int spiffs_perform_read(char *filename, FILE **f, int *file_size, char **file_buff, cJSON **json);
static int spiffs_perform_write(char *filename, FILE **f, int data_size, char *file_buff);
static int json_add_wifi_record(cJSON **json, spiffs_wifi_record_t *data, char **output_string);

//const static char string_to_encrypt[] = "string do zaszyfrowania";
//
//static unsigned char input[128], output[128];

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static char wifi_pass_file_path[32], wifi_pass_backup_file_path[32];
static EventGroupHandle_t spiffs_files_status;
static QueueHandle_t spiffs_queue_handle;
const static spiffs_routine spiffs_routines_tab[] = {

		[spiffs_check_wifi_pass_file] = spiffs_check_wifi_pass_file_routine,
		[spiffs_check_record] = spiffs_check_record_routine,
		[spiffs_add_record] = spiffs_add_record_routine,
};

/******************************************************************************************************************
 *
 * SPIFFS task
 *
 ******************************************************************************************************************/
void SPIFFS_Task(void *arg){

	spiffs_wifi_record_t *a, *b;
	BaseType_t ret;
	struct spiffs_queue_data data;

	spiffs_files_status = xEventGroupCreate();
	assert(spiffs_files_status);

	// create spiffs routines queue
	spiffs_queue_handle = xQueueCreate(3U, sizeof(struct spiffs_queue_data));
	assert(spiffs_queue_handle);

	spiffs_mount();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SPIFFS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	vTaskDelay(pdMS_TO_TICKS(1000));
	spiffs_routine_request(spiffs_check_wifi_pass_file, NULL, false);

	a = malloc(sizeof(spiffs_wifi_record_t));
	if(a){

		a->ssid = "wifi1234";
		a->iv = "vdfbdbfdsf";
		a->pass = "lbndfrbdndbdbfdb";
		a->org_len = 35;
	}
	spiffs_routine_request(spiffs_add_record, a, false);

	b = malloc(sizeof(spiffs_wifi_record_t));
	if(b){

		b->ssid = "wifi5678";
		b->iv = "dfvslkfnsvlfnslv";
		b->pass = "sdivbsidjbsvsvbfdfngdgn";
		b->org_len = 55;
	}
	spiffs_routine_request(spiffs_add_record, b, false);

	while(1){

		// recieve routine
		ret = xQueueReceive(spiffs_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// perform routine
			spiffs_routines_tab[data.type](data.arg);
		}
	}
}


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void SPIFFS_IsPasswordSaved(char *ssid){

	if(0 == ssid) return;
	spiffs_routine_request(spiffs_check_record, ssid, false);
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/
static void spiffs_mount(void){

	esp_err_t ret;
	size_t total = 0, used = 0;

	esp_vfs_spiffs_conf_t conf = {
	  .base_path = SPIFFS_MOUNT_PATH,
	  .partition_label = SPIFFS_PART_LABEL,
	  .max_files = 3,
	  .format_if_mount_failed = true,
	};

	ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

#ifdef FORMAT_SPIFFS
	esp_spiffs_format(conf.partition_label);	//TODO zakomentować!
#endif
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if(ret != ESP_OK){

		ESP_LOGE("", "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
		esp_spiffs_format(conf.partition_label);
	}
	else{

		ESP_LOGI("", "Partition size: total: %d, used: %d", total, used);
	}
}

/* send request of routine to be performed */
static void spiffs_routine_request(spiffs_routine_t type, void *arg, bool important){

	struct spiffs_queue_data data;
	BaseType_t res;

	data.type = type;
	data.arg = arg;

	// send recieved type and argument to Wifi_Task
	if(true == important){

		res = xQueueSendToFront(spiffs_queue_handle, &data, pdMS_TO_TICKS(50));
	}
	else{

		res = xQueueSend(spiffs_queue_handle, &data, pdMS_TO_TICKS(50));
	}
	if(pdPASS != res){

		// free resources if queue is full
		if(arg){
			if(heap_caps_get_allocated_size(arg)) free(arg);
		}
	}
}

static void spiffs_check_wifi_pass_file_routine(void *arg){

	FILE *f = 0;
	int file_size;
	char *wifi_pass_json_raw = 0;
	cJSON *wifi_pass_json = 0;

	xEventGroupClearBits(spiffs_files_status, WIFI_PASS_FILE_EXIST_BIT);

	// prepare file paths
	sprintf(wifi_pass_file_path, "%s%s", SPIFFS_MOUNT_PATH, SPIFFS_WIFI_PASS_FILENAME);
	sprintf(wifi_pass_backup_file_path, "%s%s", SPIFFS_MOUNT_PATH, SPIFFS_WIFI_PASS_BCKUP_FILENAME);

	// try to open the file
	f = fopen(wifi_pass_file_path, "r");
	if(0 == f){

		switch(errno){

		// if file doesn't exist
		case ENOENT:

			ESP_LOGE("", "%s file doesn't exist. Creating empty file...", wifi_pass_file_path);
			f = fopen(wifi_pass_file_path, "w");	// create empty file
			if(0 == f) goto filesystem_error;
			fclose(f);
			f = fopen(wifi_pass_file_path, "r");	// try to open once again
			if(0 == f) goto filesystem_error;
			break;

		default:
			goto filesystem_error;
			break;
		}
	}

	xEventGroupSetBits(spiffs_files_status, WIFI_PASS_FILE_EXIST_BIT);

	// read data from file and parse if file is not empty
	if(0 != spiffs_perform_read(NULL, &f, &file_size, &wifi_pass_json_raw, &wifi_pass_json)) goto error;
	if(0 != file_size){

		cJSON_Delete(wifi_pass_json);
		free(wifi_pass_json_raw);
	}

	fclose(f);
	return;

	error:
		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
		if(wifi_pass_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
		}
		if(f) fclose(f);
		ESP_LOGE("", "Error checking %s file", wifi_pass_file_path);
		if(0 == spiffs_restore_wifi_pass_backup_file()) return;

	filesystem_error:
		xEventGroupClearBits(spiffs_files_status, WIFI_PASS_FILE_EXIST_BIT);
		ESP_LOGE("", "Critical filesystem error, %s file couldn't be opened, errno = %d", wifi_pass_file_path, errno);
		ESP_LOGI("", "formating %s partition...", SPIFFS_PART_LABEL);
		esp_spiffs_format(SPIFFS_PART_LABEL);
		return;
}

static void spiffs_check_record_routine(void *arg){

//	char *ssid = (char *)arg;
//	FILE *f = 0;
//	int file_size, a;
//	char *wifi_pass_json_raw = 0;
//	cJSON *wifi_pass_json = 0;
//	EventBits_t bits;
//	WifiCreds_t *creds = 0;
//
//	bits = xEventGroupGetBits(spiffs_files_status);
//	if(!(bits & WIFI_PASS_FILE_EXIST_BIT)) goto error;
//
//	// open wifi_pass file
//	f = fopen(wifi_pass_file_path, "r");
//	if(0 == f) goto error;
//
//	// prepare return data
//	creds = calloc(1, sizeof(WifiCreds_t));
//	if(0 == creds) goto error;
//	a = strnlen(ssid, 33);
//	if(33 == a) goto error;
//	creds->ssid = malloc(a + 1);
//	if(0 == creds->ssid) goto error;
//	memcpy(creds->ssid, ssid, a + 1);
//
//	// check file size
//	fseek(f, 0, SEEK_END);
//	file_size = ftell(f);
//	fseek(f, 0, SEEK_SET);
//	if(0 == file_size){
//
//		creds->pass = 0;
//		UI_ReportEvt(UI_EVT_WIFI_REPORT_PASSWORD, creds);
//		fclose(f);
//	}
//
//	error:
//		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
//		if(wifi_pass_json_raw){
//			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
//		}
//		if(creds){
//			if(creds->ssid){
//				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
//			}
//			if(creds->pass){
//				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
//			}
//			if(heap_caps_get_allocated_size(creds)) free(creds);
//		}
//		if(f) fclose(f);
//		spiffs_check_wifi_pass_file();
}

static void spiffs_add_record_routine(void *arg){

	spiffs_add_wifi_record((spiffs_wifi_record_t *)arg);
}



/*///////////////////////////////////////////////////
 *
 * Helpers
 *
 * */////////////////////////////////////////////////

/* try to restore backup of wifi_pass file */
static int spiffs_restore_wifi_pass_backup_file(void){

	FILE *f = 0;
	int file_size = 0;
	char *wifi_pass_backup_json_raw = 0;
	cJSON *wifi_pass_backup_json = 0;

	ESP_LOGI("", "trying to restore backup file with wifi passwords...");

	// open, read, and parse wifi_pass_backup file
	if(0 != spiffs_perform_read(wifi_pass_backup_file_path, &f, &file_size, &wifi_pass_backup_json_raw,
			&wifi_pass_backup_json)) goto error;

	if(0 == file_size) goto error;

	cJSON_Delete(wifi_pass_backup_json);
	fclose(f);

	// write the data to newly created wifi_pass file
	if(0 != spiffs_perform_write(wifi_pass_file_path, &f, file_size, wifi_pass_backup_json_raw)) goto error;

	fclose(f);
	free(wifi_pass_backup_json_raw);
	ESP_LOGI("", "%s file restored", wifi_pass_backup_file_path);
	return 0;

	error:
		if(wifi_pass_backup_json) cJSON_Delete(wifi_pass_backup_json);
		if(wifi_pass_backup_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_backup_json_raw)) free(wifi_pass_backup_json_raw);
		}
		if(f) fclose(f);
		ESP_LOGE("", "%s file could not be restored. Creating empty %s file", wifi_pass_backup_file_path, wifi_pass_file_path);
		f = fopen(wifi_pass_file_path, "w");	// create empty wifi_pass file
		if(0 == f) return -1;					// shouldn't happen, return -1 to format partition
		else{

			fclose(f);
			return 0;
		}
}

/* Read wifi_pass file, parse json, add new record, save json as string
 * and save back to file. Function operates with encrypted values of data */
static void spiffs_add_wifi_record(spiffs_wifi_record_t *data){

	if(0 == data) return;

	FILE *f = 0;
	int file_size, a;
	char *wifi_pass_json_raw = 0;
	cJSON *wifi_pass_json = 0;
	EventBits_t bits;

	// if file doesn't exists
	bits = xEventGroupGetBits(spiffs_files_status);
	if(!(bits & WIFI_PASS_FILE_EXIST_BIT)) goto error;

	// open, read, and parse wifi_pass file
	if(0 != spiffs_perform_read(wifi_pass_file_path, &f, &file_size, &wifi_pass_json_raw,
			&wifi_pass_json)) goto error;
	if(0 == file_size){

		// create empty json if file has no data
		wifi_pass_json = cJSON_CreateObject();
		if(0 == wifi_pass_json) goto error;
	}
	else{

		fclose(f);

		// write backup to wifi-pass_backup file
		if(0 != spiffs_perform_write(wifi_pass_backup_file_path, &f, file_size, wifi_pass_json_raw)) goto error;

		free(wifi_pass_json_raw);
		wifi_pass_json_raw = 0;
	}

	fclose(f);

	if(0 != json_add_wifi_record(&wifi_pass_json, data, &wifi_pass_json_raw)) goto error;

	// get lenght of the string
	a = strlen(wifi_pass_json_raw);
	ESP_LOGI("", "a = %d", a);

#ifdef ZJEB_DANE
	a -= 5;
#endif

	if(0 != spiffs_perform_write(wifi_pass_file_path, &f, (a + 1), wifi_pass_json_raw)) goto error;

	// cleanup
	fclose(f);
	cJSON_Delete(wifi_pass_json);
	free(wifi_pass_json_raw);
	free(data);
	return;

	error:
		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
		if(wifi_pass_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
		}
		if(f) fclose(f);
		if(data){
			if(heap_caps_get_allocated_size(data)) free(data);
		}
		spiffs_routine_request(spiffs_check_wifi_pass_file, NULL, true);
}


static int spiffs_perform_read(char *filename, FILE **f, int *file_size, char **file_buff, cJSON **json){

	int a;

	if((0 != filename) && (0 != f)){

		// open file to read the content
		*f = fopen(filename, "r");
		if(0 == *f) return -1;
	}

	if((0 != f) && (0 != file_size)){

		// check file size
		fseek(*f, 0, SEEK_END);
		*file_size = ftell(*f);
		fseek(*f, 0, SEEK_SET);
		if(0 == *file_size) return 0;
	}

	if((0 != file_size) && (0 != file_buff)){

		// allocate memory to read file content
		*file_buff = malloc(*file_size);
		if(0 == *file_buff) return -1;
	}

	if((0 != file_size) && (0 != file_buff) && (0 != f)){

		// read file
		a = fread(*file_buff, 1, *file_size, *f);
		if(a != *file_size) return -1;
	}

	if((0 != file_buff) && (0 != json)){

		// check if the data is correct (json)
		*json = cJSON_Parse(*file_buff);
		if(0 == *json) return -1;
	}

	return 0;
}

static int spiffs_perform_write(char *filename, FILE **f, int data_size, char *file_buff){

	int a;

	if((0 != filename) && (0 != f)){

		// open wifi_pass as empty file
		*f = fopen(wifi_pass_file_path, "w");
		if(0 == *f) return -1;
	}

	if((0 < data_size) && (0 != file_buff) && (0 != f)){

		// write the string to the file
		a = fwrite(file_buff, 1, data_size, *f);
		if(a != data_size) return -1;
	}

	return 0;
}

static int json_add_wifi_record(cJSON **json, spiffs_wifi_record_t *data, char **output_string){

	cJSON *ssid = 0, *iv = 0, *pass = 0, *org_len = 0;

	// create new object with ssid as name
	ssid = cJSON_CreateObject();
	if(0 == ssid) return -1;
	cJSON_AddItemToObject(*json, data->ssid, ssid);

	// add input vector as string to the object
	iv = cJSON_CreateString(data->iv);
	if(0 == iv) return -1;
	cJSON_AddItemToObject(ssid, JSON_IV_LABEL, iv);

	// add password as string to the object
	pass = cJSON_CreateString(data->pass);
	if(0 == pass) return -1;
	cJSON_AddItemToObject(ssid, JSON_PASS_LABEL, pass);

	// add pasword length as number to the object
	org_len = cJSON_CreateNumber(data->org_len);
	if(0 == org_len) return -1;
	cJSON_AddItemToObject(ssid, JSON_ORG_LEN_LABEL, org_len);

	// convert created json to string
	*output_string = cJSON_PrintUnformatted(*json);
	if(0 == *output_string) return -1;

	return 0;
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
