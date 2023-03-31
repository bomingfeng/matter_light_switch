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

#pragma once
#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
#include <led_driver.h>
#include "iot_button.h"

#ifdef __cplusplus
extern "C" {
#endif

led_driver_handle_t led_driver_init_c3(led_driver_config_t *config);
button_config_t button_driver_get_config_c3(void);
esp_err_t led_driver_set_power_c3(led_driver_handle_t handle, bool power);

#ifdef __cplusplus
}
#endif
