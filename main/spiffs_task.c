#include "main.h"
#include "esp_event.h"
#include "esp_spiffs.h"
#include "esp_efuse.h"
#include "esp_random.h"
#include "mbedtls/aes.h"
#include <string.h>
#include "errno.h"
#include "nvs.h"

#include "cJSON.h"

//#define FORMAT_SPIFFS

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/

// bits
#define NVS_READY_BIT								(1U << 0)
#define WIFI_PASS_FILE_EXIST_BIT					(1U << 1)
#define ALARMS_FILE_EXIST_BIT						(1U << 2)

// NVS
#define NVS_CONFIG_NAMESPACE						"config"
#define NVS_KEY_THEME_NAME							"theme"
#define NVS_KEY_BACKLIGHT_NAME						"bckl"

#define NVS_WRITE_OPERATION_TIMEOUT_MS				100U

// SPIFFS
#define SPIFFS_MOUNT_PATH 							"/spiffs"
#define SPIFFS_PART_LABEL 							"spiffs"

// Wifi passwords file
#define WIFI_PASS_FILENAME							"/wifi_pass"
#define WIFI_PASS_BCKUP_FILENAME					"/wifi_pass_backup"
#define WIFI_PASS_FILE_MOUNT_PATH					(SPIFFS_MOUNT_PATH "" WIFI_PASS_FILENAME)
#define WIFI_PASS_BACKUP_FILE_MOUNT_PATH			(SPIFFS_MOUNT_PATH "" WIFI_PASS_BCKUP_FILENAME)

#define WIFI_PASS_JSON_IV_LABEL						"iv"
#define WIFI_PASS_JSON_PASS_LABEL					"pass"
#define WIFI_PASS_JSON_ORG_LEN_LABEL				"pass_len"

// Alarms file
#define ALARMS_FILENAME								"/alarms"
#define ALARMS_BCKUP_FILENAME						"/alarms_backup"
#define ALARMS_FILE_MOUNT_PATH						(SPIFFS_MOUNT_PATH "" ALARMS_FILENAME)
#define ALARMS_BACKUP_FILE_MOUNT_PATH				(SPIFFS_MOUNT_PATH "" ALARMS_BCKUP_FILENAME)

#define ALARM_0_LABEL								"alarm0"
#define ALARM_1_LABEL								"alarm1"
#define ALARM_2_LABEL								"alarm2"
#define ALARM_3_LABEL								"alarm3"

#define ALARM_HOUR_VALUE_LABEL						"hour"
#define ALARM_MINUTE_VALUE_LABEL					"min"
#define ALARM_FLAGS_VALUE_LABEL						"flags"
#define ALARM_STATUS_VALUE_LABEL					"status"
#define ALARM_TEXT_VALUE_LABEL						"text"

/**************************************************************
 *
 *	Typedefs
 *
 ***************************************************************/

// general task resources
typedef enum {

	spiffs_check_file = 0,
	spiffs_get_pass,
	spiffs_save_pass,
	spiffs_delete_pass,
	spiffs_update_alarm,
	nvs_set_config,

} spiffs_nvs_spiffs_nvs_task_routine_t;

typedef void(*spiffs_nvs_task_routine)(void *arg);

struct spiffs_nvs_task_queue_data {

	spiffs_nvs_spiffs_nvs_task_routine_t type;
	void *arg;

};

// NVS
typedef enum {

	nvs_data_string = 0, nvs_data_u8t, nvs_data_u32t,

} nvs_data_type_t;

struct nvs_pair {

	NVS_ConfigType_t config_type;
	nvs_data_type_t data_type;

	const char *key;

	union {

		char *str;
		uint8_t u8t;
		uint32_t u32t;

	} value;
};

// SPIFFS
struct spiffs_file_data {

	char *filename;
	char *backup_filename;
	uint32_t file_bit;
};

struct spiffs_alarm_data {

	uint8_t idx;
	AlarmData_t *data;
};
/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void spiffs_mount(void);
static void spiffs_nvs_task_routine_request(spiffs_nvs_spiffs_nvs_task_routine_t type, void *arg, bool important);

static void spiffs_check_file_routine(void *arg);
static void spiffs_get_pass_and_connect_routine(void *arg);
static void spiffs_save_pass_routine(void *arg);
static void spiffs_delete_pass_routine(void *arg);
static void spiffs_load_alarms_routine(void *arg);
static void spiffs_update_alarm_routine(void *arg);

static void nvs_config_load(void);
static void nvs_set_config_routine(void *arg);

static int spiffs_restore_backup_file(const struct spiffs_file_data *file);
static int spiffs_perform_read(char *filename, FILE **f, int *file_size, char **file_buff, cJSON **json);
static int spiffs_perform_write(char *filename, FILE **f, int data_size, char *file_buff);
static int json_add_wifi_record_encrypted(cJSON **json, WifiCreds_t *data, char **output_string);
static int json_get_wifi_pass_encrypted(cJSON *json, char **pass);
static int json_delete_wifi_record(cJSON **json, char *ssid, char **output_string);
static int json_create_default_alarms_file(cJSON **json, char **output_string);
static int json_restore_alarms(cJSON *json);
static int json_update_alarm(struct spiffs_alarm_data *alarm, cJSON **json, char **output_string);
static int json_fill_with_alarm_values(cJSON *json_alarm_label, AlarmData_t *alarm);
static int nvs_get_default_string(struct nvs_pair *data);
static int nvs_update_ram_config_string(struct nvs_pair *data);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/

// general
static EventGroupHandle_t spiffs_nvs_evtgroup_handle;
static QueueHandle_t spiffs_nvs_queue_handle;

static const spiffs_nvs_task_routine spiffs_nvs_task_routines_tab[] = {

		[spiffs_check_file] = spiffs_check_file_routine,
		[spiffs_get_pass] = spiffs_get_pass_and_connect_routine,
		[spiffs_save_pass] = spiffs_save_pass_routine,
		[spiffs_delete_pass] = spiffs_delete_pass_routine,
		[spiffs_update_alarm] = spiffs_update_alarm_routine,
		[nvs_set_config] = nvs_set_config_routine,
};

// NVS
static struct nvs_pair *config;
static uint8_t config_elements;

static const struct nvs_pair default_config[] = {

		[CONFIG_THEME] = {

				.config_type = CONFIG_THEME,
				.data_type = nvs_data_string,
				.key = NVS_KEY_THEME_NAME,
				.value.str = DEFAULT_THEME_NAME,
		},

		[CONFIG_BACKLIGHT] = {

				.config_type = CONFIG_BACKLIGHT,
				.data_type = nvs_data_u8t,
				.key = NVS_KEY_BACKLIGHT_NAME,
				.value.u8t = DEFAULT_BACKLIGHT,
		},

		{
				.key = NULL,	// end of array
		},
};

// SPIFFS
static const struct spiffs_file_data wifi_pass_file = {

	.filename = WIFI_PASS_FILE_MOUNT_PATH,
	.backup_filename = WIFI_PASS_BACKUP_FILE_MOUNT_PATH,
	.file_bit = WIFI_PASS_FILE_EXIST_BIT,
};

static const struct spiffs_file_data alarms_file = {

	.filename = ALARMS_FILE_MOUNT_PATH,
	.backup_filename = ALARMS_BACKUP_FILE_MOUNT_PATH,
	.file_bit = ALARMS_FILE_EXIST_BIT,
};

static const char *alarm_labels[ALARMS_NUMBER] = {

		ALARM_0_LABEL,
		ALARM_1_LABEL,
		ALARM_2_LABEL,
		ALARM_3_LABEL,
};

/******************************************************************************************************************
 *
 * SPIFFS task
 *
 ******************************************************************************************************************/
void SPIFFS_NVS_Task(void *arg){

	BaseType_t ret;
	struct spiffs_nvs_task_queue_data data;

	spiffs_nvs_evtgroup_handle = xEventGroupCreate();
	assert(spiffs_nvs_evtgroup_handle);

	// create spiffs routines queue
	spiffs_nvs_queue_handle = xQueueCreate(3U, sizeof(struct spiffs_nvs_task_queue_data));
	assert(spiffs_nvs_queue_handle);

	spiffs_mount();

	nvs_config_load();

	// wait for synchronization
	xEventGroupSync(AppStartSyncEvt, SPIFFS_NVS_TASK_BIT, ALL_TASKS_BITS, portMAX_DELAY);

	spiffs_check_file_routine((void *)&wifi_pass_file);
	spiffs_check_file_routine((void *)&alarms_file);
	spiffs_load_alarms_routine(NULL);

	while(1){

		// recieve routine
		ret = xQueueReceive(spiffs_nvs_queue_handle, &data, portMAX_DELAY);
		if(pdTRUE == ret){

			// perform routine
			spiffs_nvs_task_routines_tab[data.type](data.arg);
		}
	}
}


/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/
void SPIFFS_GetPassAndConnect(char *ssid){

	if(0 == ssid) return;
	spiffs_nvs_task_routine_request(spiffs_get_pass, ssid, false);
}

void SPIFFS_SavePass(WifiCreds_t *creds){

	if(0 == creds) return;
	spiffs_nvs_task_routine_request(spiffs_save_pass, creds, false);
}

void SPIFFS_DeletePass(char *ssid){

	if(0 == ssid) return;
	spiffs_nvs_task_routine_request(spiffs_delete_pass, ssid, false);
}

void SPIFFS_UpdateAlarm(uint8_t idx, AlarmData_t *alarm){

	if((idx >= ALARMS_NUMBER) || (0 == alarm)) return;

	struct spiffs_alarm_data *alarm_to_update;

	alarm_to_update = calloc(1U, sizeof(struct spiffs_alarm_data));
	if(0 == alarm_to_update){

		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}

		return;
	}

	alarm_to_update->idx = idx;
	alarm_to_update->data = alarm;

	spiffs_nvs_task_routine_request(spiffs_update_alarm, alarm_to_update, false);
}

void NVS_SetConfig(NVS_ConfigType_t type, void *arg){

	if((0 == config) || (type >= config_elements)) return;

	struct nvs_pair *data;
	size_t len = 0;

	// create return data
	data = calloc(1, sizeof(struct nvs_pair));
	if(0 == data) return;

	// get data type from config
	data->config_type = type;
	data->data_type = config[type].data_type;
	data->key = config[type].key;
	switch(data->data_type){

		// if string
		case nvs_data_string:

			// to set new string value, *arg must not be NULL,
			// to set default string value, *arg must be NULL
			// because set config will be performed by SPIFFS task in undefined time, we should copy
			// the value to the new buffer and free it when job is done, *arg will be freed by the taskk calling
			// SPIFFS_SetConfig function

			if(NULL != arg){

				len = strlen((char *)arg);
				if(0 == len) {

					free(data);
					return;
				}

				data->value.str = calloc((len + 1), sizeof(char));
				if(0 == data->value.str) {

					free(data);
					return;
				}

				memcpy(data->value.str, (char *)arg, (len + 1));
			}
			else{

				data->value.str = NULL;
			}

			xEventGroupClearBits(spiffs_nvs_evtgroup_handle, NVS_READY_BIT);
			spiffs_nvs_task_routine_request(nvs_set_config, data, false);
			break;

		case nvs_data_u32t:

			break;

		case nvs_data_u8t:

			// just copy the value from obtained pointer
			// and send request
			data->value.u8t = *(uint8_t *)arg;
			xEventGroupClearBits(spiffs_nvs_evtgroup_handle, NVS_READY_BIT);
			spiffs_nvs_task_routine_request(nvs_set_config, data, false);
			break;

		default:
			break;
	}
}

void NVS_GetConfig(NVS_ConfigType_t type, void *arg){

	if((0 == config) || (type >= config_elements) || (0 == arg)) return;

	EventBits_t bits;

	// wait for nvs ready bit
	bits = xEventGroupWaitBits(spiffs_nvs_evtgroup_handle, NVS_READY_BIT, pdFALSE, pdTRUE,
			pdMS_TO_TICKS(NVS_WRITE_OPERATION_TIMEOUT_MS));
	if(!(bits & NVS_READY_BIT)) return;

	switch(config[type].data_type){

		// if string
		case nvs_data_string:

			char **char_ptr = (char **)arg;
			*char_ptr = config[type].value.str;
			break;

		case nvs_data_u32t:

			break;

		// if uint8_t
		case nvs_data_u8t:

			uint8_t *u8t_ptr = (uint8_t *)arg;
			*u8t_ptr = config[type].value.u8t;
			break;

		default:
			break;
	}
}

/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* mount filesystem */
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
	esp_spiffs_format(conf.partition_label);
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
static void spiffs_nvs_task_routine_request(spiffs_nvs_spiffs_nvs_task_routine_t type, void *arg, bool important){

	struct spiffs_nvs_task_queue_data data;
	BaseType_t res;

	data.type = type;
	data.arg = arg;

	// send recieved type and argument to SPIFFS_Task
	if(true == important){

		res = xQueueSendToFront(spiffs_nvs_queue_handle, &data, pdMS_TO_TICKS(50));
	}
	else{

		res = xQueueSend(spiffs_nvs_queue_handle, &data, pdMS_TO_TICKS(50));
	}
	if(pdPASS != res){

		// free resources if queue is full
		if(arg){
			if(heap_caps_get_allocated_size(arg)) free(arg);
		}
	}
}

/* check if file exists and is correct */
static void spiffs_check_file_routine(void *arg){

	FILE *f = 0;
	int file_size;
	char *file_content_raw = 0;
	cJSON *file_json = 0;
	const struct spiffs_file_data *file = (const struct spiffs_file_data *)arg;

	xEventGroupClearBits(spiffs_nvs_evtgroup_handle, file->file_bit);

	// try to open the file
	f = fopen(file->filename, "r");
	if(0 == f){

		switch(errno){

		// if file doesn't exist
		case ENOENT:

			ESP_LOGE("", "%s file doesn't exist. Creating empty file...", file->filename);
			f = fopen(file->filename, "w");	// create empty file
			if(0 == f) goto filesystem_error;
			fclose(f);
			f = fopen(file->filename, "r");	// try to open once again
			if(0 == f) goto filesystem_error;
			break;

		default:
			goto filesystem_error;
			break;
		}
	}

	xEventGroupSetBits(spiffs_nvs_evtgroup_handle, file->file_bit);

	// read data from file and parse if file is not empty
	if(0 != spiffs_perform_read(NULL, &f, &file_size, &file_content_raw, &file_json)) goto error;
	if(0 != file_size){

		// free resources and close file if everything is ok
		cJSON_Delete(file_json);
		free(file_content_raw);
	}

	fclose(f);
	return;

	// try to restore backup file is fomething is wrong
	error:
		if(file_json) cJSON_Delete(file_json);
		if(file_content_raw){
			if(heap_caps_get_allocated_size(file_content_raw)) free(file_content_raw);
		}
		if(f) fclose(f);
		ESP_LOGE("", "Error checking %s file", file->filename);
		if(0 == spiffs_restore_backup_file(file)) return;

	// format partition if critical error occured
	filesystem_error:
		xEventGroupClearBits(spiffs_nvs_evtgroup_handle, file->file_bit);
		ESP_LOGE("", "Critical filesystem error, %s file couldn't be opened, errno = %d", file->filename, errno);
		ESP_LOGI("", "formating %s partition...", SPIFFS_PART_LABEL);
		esp_spiffs_format(SPIFFS_PART_LABEL);
		return;
}

/* get password from file for recieved ssid and send to wifi task */
static void spiffs_get_pass_and_connect_routine(void *arg){

	FILE *f = 0;
	int file_size, a = 0;
	char *wifi_pass_json_raw = 0;
	cJSON *wifi_pass_json = 0, *ssid_json = 0;
	EventBits_t bits;
	char *ssid = (char *)arg;
	WifiCreds_t *creds = 0;

	// break if file with saved password doesn't exist
	bits = xEventGroupGetBits(spiffs_nvs_evtgroup_handle);
	if(!(bits & wifi_pass_file.file_bit)) goto error;

	// read and parse file
	if(0 != spiffs_perform_read(wifi_pass_file.filename, &f, &file_size, &wifi_pass_json_raw,
			&wifi_pass_json)) goto error;

	// allocate return data
	creds = calloc(1, sizeof(WifiCreds_t));
	if(0 == creds) goto error;
	a = strnlen(ssid, 33);
	if(33 == a) goto error;
	creds->ssid = malloc(a + 1);
	if(0 == creds->ssid) goto error;
	memcpy(creds->ssid, ssid, a + 1);

	// if file is empty ask UI Task for pass
	if(0 == file_size){

		creds->pass = 0;
		UI_ReportEvt(UI_EVT_WIFI_GET_PASS, creds);
		fclose(f);
		return;
	}

	// if password for recieved ssid is saved read and decrypt it
	if(true == cJSON_HasObjectItem(wifi_pass_json, creds->ssid)){

		ssid_json = cJSON_GetObjectItemCaseSensitive(wifi_pass_json, creds->ssid);
		if(0 == ssid_json) goto error;

		if(0 != json_get_wifi_pass_encrypted(ssid_json, &creds->pass)) goto error;

		Wifi_Connect(creds);
	}
	else {

		// ask UI Task for pass
		creds->pass = 0;
		UI_ReportEvt(UI_EVT_WIFI_GET_PASS, creds);
	}

	// cleanup
	cJSON_Delete(wifi_pass_json);
	free(wifi_pass_json_raw);
	fclose(f);
	return;

	error:
		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
		if(wifi_pass_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
		}
		if(creds){
			if(creds->ssid){
				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
			}
			if(creds->pass){
				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
			}
			if(heap_caps_get_allocated_size(creds)) free(creds);
		}
		if(f) fclose(f);
		spiffs_nvs_task_routine_request(spiffs_check_file, (void *)&wifi_pass_file, true);
}

/* save recieved password */
static void spiffs_save_pass_routine(void *arg){

	WifiCreds_t *creds = (WifiCreds_t *)arg;
	FILE *f = 0;
	int file_size, a;
	char *wifi_pass_json_raw = 0;
	cJSON *wifi_pass_json = 0;
	EventBits_t bits;

	// break if file doesn't exists
	bits = xEventGroupGetBits(spiffs_nvs_evtgroup_handle);
	if(!(bits & wifi_pass_file.file_bit)) goto error;

	// open, read, and parse wifi_pass file
	if(0 != spiffs_perform_read(wifi_pass_file.filename, &f, &file_size, &wifi_pass_json_raw,
			&wifi_pass_json)) goto error;
	if(0 == file_size){

		// create empty json if file has no data
		wifi_pass_json = cJSON_CreateObject();
		if(0 == wifi_pass_json) goto error;
	}
	else{

		fclose(f);

		// write backup to wifi-pass_backup file
		if(0 != spiffs_perform_write(wifi_pass_file.backup_filename, &f, file_size, wifi_pass_json_raw)) goto error;

		free(wifi_pass_json_raw);
		wifi_pass_json_raw = 0;
	}

	fclose(f);

	// if no any password for given ssid was saved before
	if(false == cJSON_HasObjectItem(wifi_pass_json, creds->ssid)){

		// create id and save encrypted password
		if(0 != json_add_wifi_record_encrypted(&wifi_pass_json, creds, &wifi_pass_json_raw)) goto error;

		// write the modified content to a file
		a = strlen(wifi_pass_json_raw);
		if(0 != spiffs_perform_write(wifi_pass_file.filename, &f, (a + 1), wifi_pass_json_raw)) goto error;

		fclose(f);
		free(wifi_pass_json_raw);
	}

	// cleanup
	cJSON_Delete(wifi_pass_json);
	free(creds->ssid);
	free(creds->pass);
	free(creds);
	return;

	error:
		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
		if(wifi_pass_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
		}
		if(f) fclose(f);
		if(creds){
			if(creds->ssid){
				if(heap_caps_get_allocated_size(creds->ssid)) free(creds->ssid);
			}
			if(creds->pass){
				if(heap_caps_get_allocated_size(creds->pass)) free(creds->pass);
			}
			if(heap_caps_get_allocated_size(creds)) free(creds);
		}
		spiffs_nvs_task_routine_request(spiffs_check_file, (void *)&wifi_pass_file, true);
}

/* delete record with given SSID from file */
static void spiffs_delete_pass_routine(void *arg){

	char *ssid = (char *)arg;
	FILE *f = 0;
	int file_size, a;
	char *wifi_pass_json_raw = 0;
	cJSON *wifi_pass_json = 0;
	EventBits_t bits;

	// break if file with saved password doesn't exist
	bits = xEventGroupGetBits(spiffs_nvs_evtgroup_handle);
	if(!(bits & wifi_pass_file.file_bit)) goto error;

	// read and parse file
	if(0 != spiffs_perform_read(wifi_pass_file.filename, &f, &file_size, &wifi_pass_json_raw,
			&wifi_pass_json)) goto error;

	// if file is empty inform UI and return
	if(0 == file_size){

		UI_ReportEvt(UI_EVT_WIFI_PASS_NOT_DELETED, ssid);
		fclose(f);
		return;
	}
	else{

		fclose(f);

		// write backup to wifi-pass_backup file
		if(0 != spiffs_perform_write(wifi_pass_file.backup_filename, &f, file_size, wifi_pass_json_raw)) goto error;

		free(wifi_pass_json_raw);
		wifi_pass_json_raw = 0;
	}

	fclose(f);

	// if file contains item with given SSID delete id and write file back
	if(true == cJSON_HasObjectItem(wifi_pass_json, ssid)){

		if(0 != json_delete_wifi_record(&wifi_pass_json, ssid, &wifi_pass_json_raw)) goto error;

		// write the modified content to a file
		a = strlen(wifi_pass_json_raw);
		if(0 != spiffs_perform_write(wifi_pass_file.filename, &f, (a + 1), wifi_pass_json_raw)) goto error;

		fclose(f);
		free(wifi_pass_json_raw);

		UI_ReportEvt(UI_EVT_WIFI_PASS_DELETED, ssid);
	}
	else{

		UI_ReportEvt(UI_EVT_WIFI_PASS_NOT_DELETED, ssid);
	}

	// cleanup
	cJSON_Delete(wifi_pass_json);
	return;

	error:
		if(wifi_pass_json) cJSON_Delete(wifi_pass_json);
		if(wifi_pass_json_raw){
			if(heap_caps_get_allocated_size(wifi_pass_json_raw)) free(wifi_pass_json_raw);
		}
		if(ssid){
			if(heap_caps_get_allocated_size(ssid)) free(ssid);
		}
		if(f) fclose(f);
		spiffs_nvs_task_routine_request(spiffs_check_file, (void *)&wifi_pass_file, true);
}

/* load stored alarms from SPIFFS to RAM */
static void spiffs_load_alarms_routine(void *arg){

	FILE *f = 0;
	int file_size, a;
	char *alarms_file_raw = 0;
	cJSON *json_alarms = 0;
	EventBits_t bits;

	// break if file with alarms doesn't exist
	bits = xEventGroupGetBits(spiffs_nvs_evtgroup_handle);
	if(!(bits & alarms_file.file_bit)) goto error;

	// read and parse file
	if(0 != spiffs_perform_read(alarms_file.filename, &f, &file_size, &alarms_file_raw,
			&json_alarms)) goto error;

	// create file with default values if is empty
	if(0 == file_size){

		fclose(f);

		 if(0 != json_create_default_alarms_file(&json_alarms, &alarms_file_raw)) goto error;

		a = strlen(alarms_file_raw);
		if(0 != spiffs_perform_write(alarms_file.filename, &f, (a + 1), alarms_file_raw)) goto error;
	}
	fclose(f);
	free(alarms_file_raw);

	// load the data to RAM
	 if(0 != json_restore_alarms(json_alarms)) goto error;

	// cleanup
	cJSON_Delete(json_alarms);
	return;

	error:
		if(json_alarms) cJSON_Delete(json_alarms);
		if(alarms_file_raw){
			if(heap_caps_get_allocated_size(alarms_file_raw)) free(alarms_file_raw);
		}
		if(f) fclose(f);
		spiffs_nvs_task_routine_request(spiffs_check_file, (void *)&alarms_file, true);
}

/* update SPIFFS with new alarm values */
static void spiffs_update_alarm_routine(void *arg){

	FILE *f = 0;
	int file_size, a;
	char *alarms_file_raw = 0;
	cJSON *json_alarms = 0;
	EventBits_t bits;
	struct spiffs_alarm_data *alarm_to_update = (struct spiffs_alarm_data *)arg;

	// break if file with alarms doesn't exist
	bits = xEventGroupGetBits(spiffs_nvs_evtgroup_handle);
	if(!(bits & alarms_file.file_bit)) goto error;

	// read and parse file
	if(0 != spiffs_perform_read(alarms_file.filename, &f, &file_size, &alarms_file_raw,
			&json_alarms)) goto error;

	if(0 == file_size) goto error;
	fclose(f);

	// write backup to alarms backup file
	if(0 != spiffs_perform_write(alarms_file.backup_filename, &f, file_size, alarms_file_raw)) goto error;
	free(alarms_file_raw);
	fclose(f);

	// update parsed JSON
	if(0 != json_update_alarm(alarm_to_update, &json_alarms, &alarms_file_raw)) goto error;

	// write updated JSON to file
	a = strlen(alarms_file_raw);
	if(0 != spiffs_perform_write(alarms_file.filename, &f, (a + 1), alarms_file_raw)) goto error;

	// cleanup
	fclose(f);
	free(alarms_file_raw);
	cJSON_Delete(json_alarms);
	if(alarm_to_update->data->text){
		if(heap_caps_get_allocated_size(alarm_to_update->data->text)) free(alarm_to_update->data->text);
	}
	free(alarm_to_update->data);
	free(alarm_to_update);
	return;

	error:
		if(alarm_to_update){
			if(alarm_to_update->data){
				if(alarm_to_update->data->text){
					if(heap_caps_get_allocated_size(alarm_to_update->data->text)) free(alarm_to_update->data->text);
				}
				if(heap_caps_get_allocated_size(alarm_to_update->data)) free(alarm_to_update->data);
			}
			if(heap_caps_get_allocated_size(alarm_to_update)) free(alarm_to_update);
		}
		if(json_alarms) cJSON_Delete(json_alarms);
		if(alarms_file_raw){
			if(heap_caps_get_allocated_size(alarms_file_raw)) free(alarms_file_raw);
		}
		if(f) fclose(f);
		spiffs_nvs_task_routine_request(spiffs_check_file, (void *)&alarms_file, true);
}

/* load all configurations stored in NVS */
static void nvs_config_load(void){

	uint8_t a = 0;
	size_t len = 0;
	esp_err_t err;
    nvs_handle_t nvs_handle;

    // get number of stored configurations
	while(NULL != default_config[a].key){

		a++;
	}

	// allocate memory for config stored in NVS
	config = calloc(a, sizeof(struct nvs_pair));
	assert(config);

	// open NVS
	err = nvs_open(NVS_CONFIG_NAMESPACE, NVS_READWRITE, &nvs_handle);
	ESP_ERROR_CHECK(err);

	config_elements = a;

	// load all configurations from NVS to RAM "config" structure
	do{

		a--;

		// copy key name and data type from default config
		config[a].key = default_config[a].key;
		config[a].data_type = default_config[a].data_type;
		config[a].config_type = default_config[a].config_type;

		// copy the value in according to data type
		switch(config[a].data_type){

			// if string
			case nvs_data_string:

				// get length of the string
				err = nvs_get_str(nvs_handle, config[a].key, NULL, &len);
				if((ESP_FAIL == err) || (ESP_ERR_NVS_NOT_FOUND == err)){

					// if not found or corrupted try to create new pair and
					// load default value
					config[a].value.str = default_config[a].value.str;
					err = nvs_set_str(nvs_handle, config[a].key, default_config[a].value.str);
					if(ESP_OK == err) nvs_commit(nvs_handle);
				}
				else if((ESP_OK == err) && (0 != len)){

					// if string was found allocate memmory in RAM and copy
					// the value
					config[a].value.str = calloc(len, sizeof(char));
					if(0 == config[a].value.str){

						config[a].value.str = default_config[a].value.str;
						break;
					}
					err = nvs_get_str(nvs_handle, config[a].key, config[a].value.str, &len);
					if(ESP_OK != err){

						if(heap_caps_get_allocated_size(config[a].value.str)) free(config[a].value.str);
						config[a].value.str = default_config[a].value.str;
					}
				}
				else{

					ESP_LOGE("spiffs_task", "nvs_get_str error! err = %d", err);
					config[a].value.str = default_config[a].value.str;
				}

				break;

			case nvs_data_u32t:

				break;

				// if uint8_t
			case nvs_data_u8t:

				// get stored value to RAM config
				err = nvs_get_u8(nvs_handle, config[a].key, &config[a].value.u8t);
				if((ESP_FAIL == err) || (ESP_ERR_NVS_NOT_FOUND == err)){

					// if key not found or cell corupted, try to create new one
					// with default value
					config[a].value.u8t = default_config[a].value.u8t;
					err = nvs_set_u8(nvs_handle, config[a].key, config[a].value.u8t);
					if(ESP_OK == err) nvs_commit(nvs_handle);
				}
				else if(ESP_OK != err){

					ESP_LOGE("spiffs_task", "nvs_get_u8t error! err = %d", err);
					config[a].value.u8t = default_config[a].value.u8t;
				}

				break;

			default:
				ESP_LOGE("spiffs_task", "nvs_config_load(), config[%d].data type out of range, reseting...", a);
				abort();
		}

	} while(a);

	nvs_close(nvs_handle);
	xEventGroupSetBits(spiffs_nvs_evtgroup_handle, NVS_READY_BIT);
}

/* set single value for obtained key */
static void nvs_set_config_routine(void *arg){

	struct nvs_pair *data = (struct nvs_pair *)arg;
	nvs_handle_t nvs_handle = 0;
	esp_err_t err;

	// open NVS
	err = nvs_open(NVS_CONFIG_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if(ESP_OK != err) goto cleanup;

	switch(data->data_type){

		// if string
		case nvs_data_string:

			// load default value if NULL
			if(NULL == data->value.str){

				if(0 != nvs_get_default_string(data)) goto cleanup;
			}

			// set value to NVS
			err = nvs_set_str(nvs_handle, data->key, data->value.str);
			if(ESP_OK != err) goto cleanup;

			// set value to RAM config
			 if(0 != nvs_update_ram_config_string(data)) goto cleanup;
			break;

		case nvs_data_u32t:

			break;

		// if uint8_t
		case nvs_data_u8t:

			// set value to NVS
			err = nvs_set_u8(nvs_handle, data->key, data->value.u8t);
			if(ESP_OK != err) goto cleanup;

			// change RAM config
			config[data->config_type].value.u8t = data->value.u8t;
			break;

		default:
			break;
	}

	// confirm changes to NVS
	nvs_commit(nvs_handle);

	cleanup:
		if(nvs_handle) nvs_close(nvs_handle);
		if(data){
			if(nvs_data_string == data->data_type){

				if(data->value.str){

					if(heap_caps_get_allocated_size(data->value.str)) free(data->value.str);
				}
			}
			if(heap_caps_get_allocated_size(data)) free(data);
		}
		xEventGroupSetBits(spiffs_nvs_evtgroup_handle, NVS_READY_BIT);
}

/*///////////////////////////////////////////////////
 *
 * Helpers
 *
 * */////////////////////////////////////////////////

/* try to restore backup of file */
static int spiffs_restore_backup_file(const struct spiffs_file_data *file){

	FILE *f = 0;
	int file_size = 0;
	char *file_content_raw = 0;
	cJSON *file_json = 0;

	ESP_LOGI("", "trying to restore backup file...");

	// open, read, and parse wifi_pass_backup file
	if(0 != spiffs_perform_read(file->backup_filename, &f, &file_size, &file_content_raw,
			&file_json)) goto error;

	if(0 == file_size) goto error;

	cJSON_Delete(file_json);
	fclose(f);

	// write the data to newly created wifi_pass file
	if(0 != spiffs_perform_write(file->filename, &f, file_size, file_content_raw)) goto error;

	fclose(f);
	free(file_content_raw);
	ESP_LOGI("", "%s file restored", file->backup_filename);
	return 0;

	error:
		if(file_json) cJSON_Delete(file_json);
		if(file_content_raw){
			if(heap_caps_get_allocated_size(file_content_raw)) free(file_content_raw);
		}
		if(f) fclose(f);
		ESP_LOGE("", "%s file could not be restored. Creating empty %s file", file->backup_filename, file->filename);
		f = fopen(file->filename, "w");	// create empty wifi_pass file
		if(0 == f) return -1;					// shouldn't happen, return -1 to format partition
		else{

			fclose(f);
			return 0;
		}
}

/* general puprose function to open, check size, read and parse file */
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

/* general purpose function to open and write new file */
static int spiffs_perform_write(char *filename, FILE **f, int data_size, char *file_buff){

	int a;

	if((0 != filename) && (0 != f)){

		// open file as empty
		*f = fopen(filename, "w");
		if(0 == *f) return -1;
	}

	if((0 < data_size) && (0 != file_buff) && (0 != f)){

		// write the string to the file
		a = fwrite(file_buff, 1, data_size, *f);
		if(a != data_size) return -1;
	}

	return 0;
}

/* add new json record with encrypted password */
static int json_add_wifi_record_encrypted(cJSON **json, WifiCreds_t *data, char **output_string){

		cJSON *json_ssid = 0, *json_iv = 0, *json_pass = 0, *json_pass_len = 0;
		mbedtls_aes_context aes_ctx;
		const esp_efuse_desc_t **aes_key_efuse;
		unsigned char aes_key[32], iv[17], input[64], output[128] = {0};
		size_t org_len, new_len;
		uint8_t y, z;
		esp_err_t ret;

		// load AES key from efuse KEY0
	    aes_key_efuse = esp_efuse_get_key(EFUSE_BLK_KEY0);
	    ret = esp_efuse_read_field_blob(aes_key_efuse, aes_key, (sizeof(aes_key) * 8));
	    if(ESP_OK != ret){

	    	ESP_LOGE("", "Failed to read efuse");
	    	goto error;
	    }

	    // generate random input vector for encrypting password
	    esp_fill_random(iv, 16U);
	    for(y = 0; y < 16; y++){

	    	// check if there is no 0 value within random numbers
	    	// it can be treated like a string breaker
	    	// set some value if is equal to 0
	    	if(0 == iv[y]){

	    		iv[y] = y + 7;
	    	}
	    }

	    // set NULL as last item in iv_key array
	    iv[16] = 0;

		// copy current iv_key value as a string to JSON object
	    // it is important to do it before calling "mbedtls_aes_crypt_cbc" function because it will change
	    // the content of iv_key anytime when is called
	    json_iv = cJSON_CreateString((char *)iv);
		if(0 == json_iv) goto error;

		// initialize context of AES crypting operation
		mbedtls_aes_init(&aes_ctx);
		mbedtls_aes_setkey_enc(&aes_ctx, aes_key, 256);

		// check length of password
		org_len = strnlen(data->pass, 64);	// len = 23
		if((0 == org_len) || (64 == org_len)) goto error;

		// copy password to input buffer
	    memcpy(input, data->pass, org_len);
		new_len = org_len;

		// AES can only encrypt data that is a multiple of 16 bytes
		// to pass this condition we have to add padding bytes with '0' character

	    z = org_len % 16U;	// how many characters are left and need to be padded	// z = 7

	    // if there are characters to be padded
	    if(0 != z){

	    	// how many bytes are needed
	    	y = 16U - z;		// y = 9
	    	while(0 != y){

	    		input[new_len] = '0';
	    		new_len++;
	    		y--;
	    	}
	    }

	    // perform password encryption
	    if(0 != mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_ENCRYPT, new_len, iv, input, output )){

	    	ESP_LOGE("", "encryption has failed");
	    	goto error;
	    }

		// create new object with ssid as name
	    json_ssid = cJSON_CreateObject();
		if(0 == json_ssid) goto error;
		cJSON_AddItemToObject(*json, data->ssid, json_ssid);

		// add iv object (created above)
		cJSON_AddItemToObject(json_ssid, WIFI_PASS_JSON_IV_LABEL, json_iv);

		// add pasword length as number to the object
		json_pass_len = cJSON_CreateNumber(org_len);
		if(0 == json_pass_len) goto error;
		cJSON_AddItemToObject(json_ssid, WIFI_PASS_JSON_ORG_LEN_LABEL, json_pass_len);

		// add password as string to the object
		json_pass = cJSON_CreateString((char *)output);
		if(0 == json_pass) goto error;
		cJSON_AddItemToObject(json_ssid, WIFI_PASS_JSON_PASS_LABEL, json_pass);

		// convert created json to string
		*output_string = cJSON_PrintUnformatted(*json);
		if(0 == *output_string) goto error;

		// cleanup
		mbedtls_aes_free(&aes_ctx);
		return 0;

		error:
			mbedtls_aes_free(&aes_ctx);
			return -1;
}

/* read and decrypt password from json record */
static int json_get_wifi_pass_encrypted(cJSON *json, char **pass){

	cJSON *json_pass = 0, *json_iv = 0, *json_pass_len = 0;
	mbedtls_aes_context aes_ctx;
	const esp_efuse_desc_t **aes_key_efuse;
	unsigned char input[128], output[65] = {0}, iv[17], aes_key[32];
	int len, org_len, a;
	esp_err_t ret;

	// copy input vector to buffer
	json_iv = cJSON_GetObjectItemCaseSensitive(json, WIFI_PASS_JSON_IV_LABEL);
	if(0 == json_iv) goto error;
	if (0 == cJSON_IsString(json_iv) || (json_iv->valuestring == 0)) goto error;

	len = strnlen(json_iv->valuestring, 17);
	if((0 == len) || (17 == len)) goto error;

	memcpy(iv, json_iv->valuestring, (len + 1));

	// copy encrypted password to input buffer
	json_pass = cJSON_GetObjectItemCaseSensitive(json, WIFI_PASS_JSON_PASS_LABEL);
	if(0 == json_pass) goto error;
	if (0 == cJSON_IsString(json_pass) || (json_pass->valuestring == 0)) goto error;

	len = strnlen(json_pass->valuestring, 128);
	if((0 == len) || (128 == len)) goto error;

	memcpy(input, json_pass->valuestring, (len + 1));

	// get password length
	json_pass_len = cJSON_GetObjectItemCaseSensitive(json, WIFI_PASS_JSON_ORG_LEN_LABEL);
	if(0 == json_pass_len) goto error;
	if(0 == cJSON_IsNumber(json_pass_len)) goto error;
	org_len = json_pass_len->valueint;

	// load AES key from efuse KEY0
    aes_key_efuse = esp_efuse_get_key(EFUSE_BLK_KEY0);
    ret = esp_efuse_read_field_blob(aes_key_efuse, aes_key, (sizeof(aes_key) * 8));
    if(ESP_OK != ret){

    	ESP_LOGE("", "Failed to read efuse");
    	goto error;
    }

	// initialize context of AES crypting operation
	mbedtls_aes_init(&aes_ctx);
	mbedtls_aes_setkey_dec(&aes_ctx, aes_key, 256);

	// perform decryption of password
	a = mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_DECRYPT, len, iv, input, output );
	if(0 != a){

		ESP_LOGE("", "decryption has failed, a = %d", a);
		goto error;
	}

	// set 0 after the last characted
    output[org_len] = 0;

    // prepare output data
	*pass = calloc(1, (org_len + 1));
	if(0 == *pass) goto error;

	memcpy(*pass, output, (org_len + 1));

	// cleanup
    mbedtls_aes_free(&aes_ctx);
	return 0;

	error:
		if(*pass){
			if(heap_caps_get_allocated_size(*pass)) free(*pass);
		}
		mbedtls_aes_free(&aes_ctx);
		return -1;
}

static int json_delete_wifi_record(cJSON **json, char *ssid, char **output_string){

	cJSON *deleted_record = cJSON_DetachItemFromObjectCaseSensitive(*json, ssid);
	if(0 == deleted_record) return -1;

	cJSON_Delete(deleted_record);

	// convert created json to string
	*output_string = cJSON_PrintUnformatted(*json);
	if(0 == *output_string) return -1;

	return 0;
}

/* create new JSON object with default values for each one alarm */
static int json_create_default_alarms_file(cJSON **json, char **output_string){

	uint8_t a;
	const AlarmData_t *alarms = 0;
	cJSON *json_alarm_label = 0;

	// load default values
	alarms = Alarm_GetDefaultValues();
	if(0 == alarms) goto error;

	// create new JSON object
	*json = cJSON_CreateObject();
	if(0 == *json) goto error;

	// create 4 alarm objects with default values
	// every object should look like this:
	/*
		"alarm0":{
			"hour":		0,
			"min": 		0,
			"flags": 	0,
			"status": 	false,
			"text": 	"-- not-set --"
    	}
    */
	for(a = 0; a < ALARMS_NUMBER; a++){

		json_alarm_label = cJSON_CreateObject();
		if(0 == json_alarm_label) goto error;
		cJSON_AddItemToObject(*json, alarm_labels[a], json_alarm_label);

		if(0 != json_fill_with_alarm_values(json_alarm_label, (AlarmData_t *)alarms)) goto error;
	}

	// convert created json to string
	*output_string = cJSON_PrintUnformatted(*json);
	if(0 == *output_string) goto error;

	return 0;

	error:
		return -1;
}

/* restore all alarms from parsed JSON to RAM */
static int json_restore_alarms(cJSON *json){

	uint8_t a;
	int len;
	AlarmData_t *alarm = 0;
	cJSON *json_alarm_label = 0, *json_alarm_hour = 0, *json_alarm_minute = 0, *json_alarm_flags = 0,
		*json_alarm_status = 0, *json_alarm_text = 0;

	// for each one alarm allocate the momory and fill with
	// data obtained from JSON
	// next fill the RAM buffer with this data -> alarm.c file
	for(a = 0; a < ALARMS_NUMBER; a++){

		alarm = calloc(1, sizeof(AlarmData_t));
		if(0 == alarm) goto error;

		json_alarm_label = cJSON_GetObjectItemCaseSensitive(json, alarm_labels[a]);
		if(0 == json_alarm_label) goto error;

		json_alarm_hour = cJSON_GetObjectItemCaseSensitive(json_alarm_label, ALARM_HOUR_VALUE_LABEL);
		if(0 == json_alarm_hour) goto error;
		if(0 == cJSON_IsNumber(json_alarm_hour)) goto error;
		alarm->hour = json_alarm_hour->valueint;

		json_alarm_minute = cJSON_GetObjectItemCaseSensitive(json_alarm_label, ALARM_MINUTE_VALUE_LABEL);
		if(0 == json_alarm_minute) goto error;
		if(0 == cJSON_IsNumber(json_alarm_minute)) goto error;
		alarm->minute = json_alarm_minute->valueint;

		json_alarm_flags = cJSON_GetObjectItemCaseSensitive(json_alarm_label, ALARM_FLAGS_VALUE_LABEL);
		if(0 == json_alarm_flags) goto error;
		if(0 == cJSON_IsNumber(json_alarm_flags)) goto error;
		alarm->flags = json_alarm_flags->valueint;

		json_alarm_status = cJSON_GetObjectItemCaseSensitive(json_alarm_label, ALARM_STATUS_VALUE_LABEL);
		if(0 == json_alarm_status) goto error;
		if(0 == cJSON_IsBool(json_alarm_status)) goto error;
		alarm->status = cJSON_IsTrue(json_alarm_status);

		json_alarm_text = cJSON_GetObjectItemCaseSensitive(json_alarm_label, ALARM_TEXT_VALUE_LABEL);
		if(0 == json_alarm_text) goto error;
		if (0 == cJSON_IsString(json_alarm_text) || (json_alarm_text->valuestring == 0)) goto error;

		len = strlen(json_alarm_text->valuestring);
		if(0 == len) goto error;

		alarm->text = malloc(len + 1);
		if(0 == alarm->text) goto error;

		memcpy(alarm->text, json_alarm_text->valuestring, (len + 1));

		Alarm_RestoreFromSPIFFS(alarm, a);
	}

	return 0;

	error:
		if(alarm){
			if(alarm->text){
				if(heap_caps_get_allocated_size(alarm->text)) free(alarm->text);
			}
			if(heap_caps_get_allocated_size(alarm)) free(alarm);
		}
		return -1;
}

/* function finds single alarm's JSON within the whole file, deletes it and creates new one with updated values*/
static int json_update_alarm(struct spiffs_alarm_data *alarm, cJSON **json, char **output_string){

	cJSON *json_alarm_label = 0;

	// find the old JSON within the file
	json_alarm_label = cJSON_DetachItemFromObjectCaseSensitive(*json, alarm_labels[alarm->idx]);
	if(0 == json_alarm_label) goto error;

	// delete the old JSON
	cJSON_Delete(json_alarm_label);

	// create new JSON
	json_alarm_label = cJSON_CreateObject();
	if(0 == json_alarm_label) goto error;
	cJSON_AddItemToObject(*json, alarm_labels[alarm->idx], json_alarm_label);

	if(0 != json_fill_with_alarm_values(json_alarm_label, alarm->data)) goto error;

	// convert created json to string
	*output_string = cJSON_PrintUnformatted(*json);
	if(0 == *output_string) goto error;
	return 0;

	error:
		return -1;
}

/* fill single alarm json with values from AlarmData_t structure */
static int json_fill_with_alarm_values(cJSON *json_alarm_label, AlarmData_t *alarm){

	cJSON *json_alarm_hour = 0, *json_alarm_minute = 0, *json_alarm_flags = 0,
		*json_alarm_status = 0, *json_alarm_text = 0;

	json_alarm_hour = cJSON_CreateNumber(alarm->hour);
	if(0 == json_alarm_hour) goto error;
	cJSON_AddItemToObject(json_alarm_label, ALARM_HOUR_VALUE_LABEL, json_alarm_hour);

	json_alarm_minute = cJSON_CreateNumber(alarm->minute);
	if(0 == json_alarm_minute) goto error;
	cJSON_AddItemToObject(json_alarm_label, ALARM_MINUTE_VALUE_LABEL, json_alarm_minute);

	json_alarm_flags = cJSON_CreateNumber(alarm->flags);
	if(0 == json_alarm_flags) goto error;
	cJSON_AddItemToObject(json_alarm_label, ALARM_FLAGS_VALUE_LABEL, json_alarm_flags);

	json_alarm_status = cJSON_CreateBool(alarm->status);
	if(0 == json_alarm_status) goto error;
	cJSON_AddItemToObject(json_alarm_label, ALARM_STATUS_VALUE_LABEL, json_alarm_status);

	json_alarm_text = cJSON_CreateString(alarm->text);
	if(0 == json_alarm_text) goto error;
	cJSON_AddItemToObject(json_alarm_label, ALARM_TEXT_VALUE_LABEL, json_alarm_text);

	return 0;

	error:
		return -1;
}

/* get default string value for obtained key */
static int nvs_get_default_string(struct nvs_pair *data){

	size_t len = 0;

	// get length of string value from default config
	len = strlen(default_config[data->config_type].value.str);
	if(0 == len) return -1;

	// allocate memory and copyt he string
	data->value.str = calloc((len + 1), sizeof(char));
	if(0 == data->value.str) return -1;

	memcpy(data->value.str, default_config[data->config_type].value.str, (len + 1));
	return 0;
}

/* update changed string value in RAM config */
static int nvs_update_ram_config_string(struct nvs_pair *data){

	size_t len = 0;

	// get length of new string
	len = strlen(data->value.str);
	if(0 == len) return -1;

	// free old data
	if(heap_caps_get_allocated_size(config[data->config_type].value.str)) free(config[data->config_type].value.str);

	// allocate and copy the new one
	config[data->config_type].value.str = calloc((len + 1), sizeof(char));
	if(0 == config[data->config_type].value.str) return -1;

	memcpy(config[data->config_type].value.str, data->value.str, (len + 1));
	return 0;
}
