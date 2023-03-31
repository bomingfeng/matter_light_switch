/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* ADC1 Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "ADC1_single_read_Task.h"

//ADC Channels
#define ADC1_EXAMPLE_CHAN0          CONFIG_Battery_Level_GPIO_PIN

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11

extern uint32_t sse_data[sse_len];

void ADC1_single_read_Task(void *pvParam)
{
  float adc_data;
  //ADC1 config
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

  while (1) {
    adc_data = 3.3 * adc1_get_raw(ADC1_EXAMPLE_CHAN0) / 4095;
    ESP_LOGI("ADC1","adc1_data:DC%fV.",adc_data);
    adc_data *= 100;
    ESP_LOGI("ADC1","adc1_data:DC%fV.",adc_data);
    sse_data[1] = adc_data;
    ESP_LOGI("ADC1","adc1_data:DC%dV.",sse_data[1]);
    if(sse_data[1] > 146){
      xEventGroupSetBits(APP_event_group,APP_event_Low_Battery_BIT);
    }
    else{
      xEventGroupClearBits(APP_event_group,APP_event_Low_Battery_BIT);
      const int wakeup_time_sec = 30;
      ESP_LOGI("ADC1","Enabling timer wakeup, %dmin", wakeup_time_sec);
      esp_sleep_enable_timer_wakeup(wakeup_time_sec * 60000000);
      ESP_LOGI("ADC1","Entering deep sleep");
      esp_deep_sleep_start();
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}