// Copyright 2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#include <driver/ledc.h>
#include <esp_log.h>
#include <hal/ledc_types.h>

#include <led_driver.h>
#include <driver_esp32c3.h>

#include "driver/gpio.h"

#include "app_inclued.h"

static const char *TAG = "led_driver_gpio";

led_driver_handle_t led_driver_init_c3(led_driver_config_t *config)
{
    gpio_reset_pin(CONFIG_Lights_GPIO);
    gpio_set_direction(CONFIG_Lights_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(CONFIG_Lights_GPIO,GPIO_PULLUP_PULLDOWN);
    gpio_set_level(CONFIG_Lights_GPIO,0);

    gpio_reset_pin(CONFIG_detectIR_GPIO);
    gpio_set_level(CONFIG_detectIR_GPIO,0);
    gpio_set_direction(CONFIG_detectIR_GPIO,GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_detectIR_GPIO,GPIO_PULLUP_PULLDOWN);

    /* Using (channel + 1) as handle */
    return (led_driver_handle_t) CONFIG_Lights_GPIO;
}



button_config_t button_driver_get_config_c3(void)
{
    button_config_t config = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = CONFIG_BUTTON_GPIO_PIN,
            .active_level = 0,
        }
    };
    return config;
}



esp_err_t led_driver_set_power_c3(led_driver_handle_t handle, bool power)
{
    if((xEventGroupGetBits(APP_event_group) & APP_event_Force_off_lights_BIT) == APP_event_Force_off_lights_BIT){
        gpio_set_level((gpio_num_t)handle,0);
    }
    else{
         gpio_set_level((gpio_num_t)handle,power);
    }
   
    return ESP_OK;
}
