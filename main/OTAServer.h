#pragma once

#include "esp_http_server.h"

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE   (200*1024) // 200 KB
#define MAX_FILE_SIZE_STR "200KB"

/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

#ifdef __cplusplus
extern "C" {
#endif
extern httpd_handle_t OTA_server;

void systemRebootTask(void * parameter);
esp_err_t start_file_server(const char *base_path);
void stop_file_server(void);

#ifdef __cplusplus
}
#endif




