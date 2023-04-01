#ifndef main_H
#define main_H

#include <stdio.h>
#include <stdlib.h>
#include <nvs_flash.h>
#include "esp_task.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "nvs.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/event_groups.h"
#include "math.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "freertos/queue.h"
#include "esp_vfs.h"
#include "sdkconfig.h"

#define sse_len 10
#define sse_Least (sse_len-1)


extern EventGroupHandle_t APP_event_group;
#define APP_event_REBOOT_BIT   BIT14 // =1 重起
#define APP_event_deepsleep_BIT     // =1 休眠5s使能


#define APP_event_Standby_BIT   // =1 空调待机
#define APP_event_run_BIT  BIT22 // =1 空调运行
#define APP_event_tcp_client_send_BIT   // =1 TCP_Client允许发送数据
#define APP_event_lighting_BIT   BIT7    // =1 空调灯光显示
#define APP_event_30min_timer_BIT BIT8   // =1 
#define APP_event_IO_wakeup_sleep_BIT BIT9  // =1 io 休眠使能
#define APP_event_io_sleep_timer_BIT BIT10 // =1 启动io_sleep_timers定时器
#define APP_event_LP_flags_BIT  // =1
#define APP_event_LLP_flags_BIT  // =1
#define APP_event_IR_LED_flags_BIT BIT14 // =1 接收到IR数据正确
#define APP_event_BLE_CONNECTED_flags_BIT BIT15 // =1 接收到ble数据正确
#define APP_event_ds18b20_CONNECTED_flags_BIT BIT16 // =1 接收到ds18b20数据正确
#define APP_event_SNTP_ok_flags_BIT BIT17 //=1 时间同步完成。
#define APP_event_Log_msg_ok_flags_BIT BIT18 // =1 可以写数据
#define APP_event_Button_SINGLE_CLICK_BIT BIT11 // =1 
#define APP_event_Button_PRESS_REPEAT_BIT BIT19   // =1 按键三按以上
#define APP_event_Button_LONG_PRESS_BIT BIT20    // =1 按键长按上
#define APP_event_time_wakeup_sleep_BIT BIT5    // =1 时间唤醒后进入休眠
#define APP_event_log_spiffs_sleep_BIT BIT21
#define APP_event_LED_light_BIT  BIT23 // =1 LED常亮
#define APP_event_next_reboot1_BIT  BIT12

#define APP_event_Low_Battery_BIT BIT5    // =1 时间唤醒后进入休眠
#define APP_event_Button_wakeup_sleep_BIT BIT4    // =1 按键唤醒后进入休眠
#define APP_event_Force_off_lights_BIT  BIT13
#define APP_event_WIFI_STA_CONNECTED_BIT  BIT6  // =1 wifi连接
#define APP_event_WIFI_AP_CONNECTED_BIT  BIT3   // =1 有设备连接本热点

#define APP_event_LED_light_Always_off()  xEventGroupClearBits(APP_event_group,BIT0 | BIT1 | BIT2)//0 常灭
#define APP_event_LED_light_Breathe()  xEventGroupClearBits(APP_event_group,BIT1 | BIT2);xEventGroupSetBits(APP_event_group,BIT0) // 1 呼吸
#define APP_event_LED_light_100ms()  xEventGroupClearBits(APP_event_group,BIT0 | BIT2);xEventGroupSetBits(APP_event_group,BIT1) // 2 
#define APP_event_LED_light_500ms()  xEventGroupClearBits(APP_event_group,BIT2);xEventGroupSetBits(APP_event_group,BIT0 | BIT1) // 3 
#define APP_event_LED_light_1s()  xEventGroupClearBits(APP_event_group,BIT0 | BIT1);xEventGroupSetBits(APP_event_group,BIT2) // 4 
#define APP_event_LED_light_1s5()  xEventGroupClearBits(APP_event_group,BIT1);xEventGroupSetBits(APP_event_group,BIT0 | BIT2) // 5 
#define APP_event_LED_light_2s()  xEventGroupClearBits(APP_event_group,BIT0);xEventGroupSetBits(APP_event_group,BIT1 | BIT2) // 5
#define APP_event_LED_light_Always_On()  xEventGroupSetBits(APP_event_group,BIT0 | BIT1 | BIT2)//7 常亮


//extern RTC_DATA_ATTR uint32_t sleep_keep;
#define sleep_keep_WIFI_AP_OR_STA_BIT BIT0
#define sleep_keep_Thermohygrometer_Low_battery_BIT  BIT1


#endif /* main_H */