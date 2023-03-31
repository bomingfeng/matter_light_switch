#ifndef log_spiffs_H
#define log_spiffs_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_inclued.h"

extern char log_msg[200];

esp_err_t init_spiffs(void);
void log_task(void * arg);


#ifdef __cplusplus
}
#endif

#endif /* log_spiffs */