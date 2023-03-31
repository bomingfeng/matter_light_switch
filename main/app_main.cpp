/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app_priv.h>
#include <app_reset.h>

#include "OTAServer.h"

#include <app/server/OnboardingCodesUtil.h>
#include "app_inclued.h"
#include "lib/shell/Commands.h"

#include "led_Task.h"
#include "log_spiffs.h"
#include "htmltomcu.h"
#include "myWiFi.h"
#include "ADC1_single_read_Task.h"

#include <esp_matter_controller_pairing_command.h>
#include <esp_matter_commissioner.h>
#include <esp_matter_controller_console.h>

static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address Changed");
        APP_event_LED_light_Breathe();
        start_file_server("/spiffs");
        xEventGroupClearBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
        xEventGroupSetBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case 0x8000:
        ESP_LOGI(TAG, "Updating advertising data:0x%x.",event->Type);
        break;

    case 0x8002:
        ESP_LOGI(TAG, "IPv4 Internet connectivity ESTABLISHED:0x%x.",event->Type);
        break;

    case 0xc000:
        APP_event_LED_light_1s();
        ESP_LOGI(TAG, "Haven't to connect to a suitable AP now!:0x%x.",event->Type);
        break;    

    default:
        ESP_LOGI(TAG, "app_event_cb : default:0x%x.",event->Type);
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %d, endpoint_id: %d, effect_id: %d", type, endpoint_id, effect_id);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    if (type == PRE_UPDATE) {
        /* Handle the attribute updates here. */
    }

    return ESP_OK;
}

static void InitServer(intptr_t context)
{
    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
}

EventGroupHandle_t APP_event_group;
extern MessageBufferHandle_t HtmlToMcuData;
nvs_handle_t my_handle;

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    wifi_mode_t enWifiMode;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    APP_event_group = xEventGroupCreate();
    HtmlToMcuData = xMessageBufferCreate(100);
    LED_Task_init();
    xTaskCreatePinnedToCore(led_instructions, "led_instructions", 4096, NULL, ESP_TASK_PRIO_MIN + 1, NULL, tskNO_AFFINITY);
    APP_event_LED_light_100ms();

    init_spiffs();
    xTaskCreatePinnedToCore(htmltomcudata_task, "htmltomcudata", 4096, NULL, ESP_TASK_PRIO_MIN + 2, NULL,tskNO_AFFINITY);           
    xTaskCreatePinnedToCore(log_task, "log_task", 4096, NULL, ESP_TASK_PRIO_MIN + 1, NULL,tskNO_AFFINITY);//0;1;tskNO_AFFINITY
    xTaskCreatePinnedToCore(ADC1_single_read_Task, "ADC1", 2048, NULL, ESP_TASK_PRIO_MIN + 1, NULL,tskNO_AFFINITY);//0;1;tskNO_AFFINITY

   	enWifiMode = WIFI_Mode_Check();
    if(WIFI_MODE_AP == WIFI_Mode_Check()){
        APP_event_LED_light_500ms();
        xTaskCreatePinnedToCore(vTaskWifiHandler, "vTaskWifiHandler", 6144, NULL, ESP_TASK_PRIO_MIN + 2, NULL,tskNO_AFFINITY);
    }  
    else if(WIFI_MODE_STA == enWifiMode){

        APP_event_LED_light_1s();
        /* Initialize driver */
        app_driver_handle_t switch_handle = app_driver_switch_init();
        app_reset_button_register(switch_handle);

        /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
        node::config_t node_config;
        node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);

        on_off_switch::config_t switch_config;
        endpoint_t *endpoint = on_off_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, switch_handle);

        /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
        if (!node || !endpoint) {
            ESP_LOGE(TAG, "Matter node creation failed");
        }

        /* Add group cluster to the switch endpoint */
        cluster::groups::config_t groups_config;
        cluster::groups::create(endpoint, &groups_config, CLUSTER_FLAG_SERVER | CLUSTER_FLAG_CLIENT);

        switch_endpoint_id = endpoint::get_id(endpoint);
        ESP_LOGI(TAG, "Switch created with endpoint_id %d", switch_endpoint_id);

        /* Matter start */
        err = esp_matter::start(app_event_cb);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Matter start failed: %d", err);
        }

    #if CONFIG_ENABLE_CHIP_SHELL
        esp_matter::console::diagnostics_register_commands();
        esp_matter::console::wifi_register_commands();
        esp_matter::console::init();
    #endif
        chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
    }
    else{
        
        //vTaskDelay(20000 / portTICK_PERIOD_MS); 
        //esp_matter::controller::pairing_on_network(/*uint64_t*/0x7823, /*uint32_t*/CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE);

        #if CONFIG_ESP_MATTER_CONTROLLER_ENABLE
        #if CONFIG_ESP_MATTER_COMMISSIONER_ENABLE
        esp_matter::lock::chip_stack_lock(portMAX_DELAY);
        esp_matter::commissioner::init(5580);
        esp_matter::lock::chip_stack_unlock();
        #endif // CONFIG_ESP_MATTER_COMMISSIONER_ENABLE
        esp_matter::console::controller_register_commands();
        #endif // CONFIG_ESP_MATTER_CONTROLLER_ENABLE
    }
}
