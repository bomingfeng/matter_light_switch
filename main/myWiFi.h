#ifndef myWiFi_H
#define myWiFi_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_inclued.h"
#include "freertos/timers.h"
#include "esp_wifi_types.h"

void WIFI_Mode_Save(wifi_mode_t WifiMode);
wifi_mode_t WIFI_Mode_Check(void);
void vTaskWifiHandler(void * pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* led_Task_H */