/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* It is recommended to copy this code in your example so that you can modify as per your application's needs,
 * especially for the indicator calbacks, button_factory_reset_pressed_cb() and button_factory_reset_released_cb().
 */

#include <esp_log.h>
#include <device.h>
#include <esp_matter.h>

#include "app_inclued.h"
//#include "myWiFi.h"


static const char *TAG = "app_reset";
static bool perform_factory_reset = false;
static uint8_t BUTTON_DOUBLE_CLICK_count = 0;
static TimerHandle_t QuitTimers;

///////////////////////////////////////////////////////////////////////////////////////////////////////
static void WIFI_Mode_Save(wifi_mode_t WifiMode)
{
    static nvs_handle_t my_handle;
	// Open
	//printf("\n");
	//printf("Opening Non-Volatile Storage (NVS) handle... ");
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		//printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		//printf("Done\n");
	}   

	// Write
	//printf("Updating restart counter in NVS ... ");
	
	
	err=nvs_set_i8(my_handle,"wifi_mode",WifiMode);
	//printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

	

	// Commit written value.
	// After setting any values, nvs_commit() must be called to ensure changes are written
	// to flash storage. Implementations may write to storage at other times,
	// but this is not guaranteed.
	//printf("Committing updates in NVS ... ");
	err = nvs_commit(my_handle);
	//printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

	// Close
	nvs_close(my_handle);
}

static void button_factory_reset_pressed_cb(void *arg, void *data)
{
    if (!perform_factory_reset) {
        ESP_LOGI(TAG, "Factory reset triggered. Release the button to start factory reset.");
        perform_factory_reset = true;
    }
}

static void button_factory_reset_released_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "button_factory_reset_released_cb:%d",BUTTON_DOUBLE_CLICK_count);
    xTimerReset(QuitTimers,portMAX_DELAY);
    BUTTON_DOUBLE_CLICK_count++;
    if(BUTTON_DOUBLE_CLICK_count == 8){
        WIFI_Mode_Save(WIFI_MODE_AP);
    }
    if(BUTTON_DOUBLE_CLICK_count >= 16){
        WIFI_Mode_Save(WIFI_MODE_AP);
        esp_matter::factory_reset();
        perform_factory_reset = false;
    }
    if (perform_factory_reset) {
        ESP_LOGI(TAG, "Starting factory reset");
        WIFI_Mode_Save(WIFI_MODE_AP);
        esp_matter::factory_reset();
        perform_factory_reset = false;
    }
}

static void vQuitTimersCallback(TimerHandle_t xTimer)
{
    xTimerStop(QuitTimers,portMAX_DELAY);
    ESP_LOGI("app_reset", "vQuitTimersCallback_1");//
    if(BUTTON_DOUBLE_CLICK_count >= 8){
        esp_restart();
    }
    BUTTON_DOUBLE_CLICK_count = 0;
}

esp_err_t app_reset_button_register(void *handle)
{
    QuitTimers = xTimerCreate("QuitTimers",1000/portTICK_PERIOD_MS,pdFALSE,( void * ) 1,vQuitTimersCallback);
            
    if (!handle) {
        ESP_LOGE(TAG, "Handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    button_handle_t button_handle = (button_handle_t)handle;
    esp_err_t err = ESP_OK;
    err |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_HOLD, button_factory_reset_pressed_cb, NULL);
    err |= iot_button_register_cb(button_handle, BUTTON_PRESS_UP, button_factory_reset_released_cb, NULL);
    return err;
}
