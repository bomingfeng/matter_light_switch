#ifndef main_H
#define main_H

#include <stdio.h>
#include <stdlib.h>
#include <nvs_flash.h>
#include "OTAServer.h"
#include "MyWiFi.h"
#include "esp_task.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "nvs.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "ds18x20.h"
#include "driver/rtc_io.h"
#include "esp_sntp.h"
#include "esp_sleep.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/event_groups.h"
#include "math.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/queue.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "log_spiffs.h"
#include "sdkconfig.h"

//#define Gree    
#define Auxgroup 

#define LYWSD03MMC    
//#define XL0801



#ifdef  Gree
#include "ir_parser_rmt_YAPOF3.h"
#define ir_parser_rmt_new(ir_parser_config) ir_parser_rmt_new_YAPOF3(ir_parser_config)
#define ir_builder_rmt_new(ir_builder_config) ir_builder_rmt_new_YB0F2(ir_builder_config)
#define RMT_CONFIG_TX(gpio, channel_id) RMT_YB0F2_CONFIG_TX(gpio, channel_id)
#define IR_BUILDER_CONFIG(dev) IR_BUILDER_YB0F2_CONFIG(dev)
#define RMT_CONFIG_RX(gpio, channel_id) RMT_YB0F2_CONFIG_RX(gpio, channel_id)
#define IR_PARSER_CONFIG(dev) IR_PARSER_YB0F2_CONFIG(dev)
#endif

#ifdef  Auxgroup
#include "ir_parser_rmt_YKR_T_091.h"
#define ir_parser_rmt_new(ir_parser_config) ir_parser_rmt_new_YKR_T_091(ir_parser_config)
#define ir_builder_rmt_new(ir_builder_config) ir_builder_rmt_new_YKR_T_091(ir_builder_config)
#define RMT_CONFIG_TX(gpio, channel_id) RMT_YKR_T_091_CONFIG_TX(gpio, channel_id)
#define IR_BUILDER_CONFIG(dev) IR_BUILDER_YKR_T_091_CONFIG(dev)
#define RMT_CONFIG_RX(gpio, channel_id) RMT_YKR_T_091_CONFIG_RX(gpio, channel_id)
#define IR_PARSER_CONFIG(dev) IR_PARSER_YKR_T_091_CONFIG(dev)
#endif



#define GATTC_TAG "LYWSD03MMC"
#define Sp 18 //温度上、下限
#define Lp 15
#define LLp 25

#ifdef XL0801
#define load_time 1 //多久控制空调
#else
#define load_time 45//多久控制空调
#endif

#define sleep_time 5  //休眠时间min
#define time_off 500 //默认定时关空调时间MIN
#define BLe_battery_low 1810 //2110
#define BLe_battery_High 2320

#define sse_len 10
#define sse_Least (sse_len-1)


extern EventGroupHandle_t APP_event_group;
#define APP_event_REBOOT_BIT BIT23   // =1 重起
#define APP_event_deepsleep_BIT BIT11    // =1 休眠5s使能
#define APP_event_WIFI_STA_CONNECTED_BIT  BIT12  // =1 wifi连接
#define APP_event_WIFI_AP_CONNECTED_BIT  BIT13   // =1 有设备连接本热点
#define APP_event_Standby_BIT  BIT21 // =1 空调待机
#define APP_event_run_BIT  BIT22 // =1 空调运行
#define APP_event_tcp_client_send_BIT  BIT6 // =1 TCP_Client允许发送数据
#define APP_event_lighting_BIT   BIT7    // =1 空调灯光显示
#define APP_event_30min_timer_BIT BIT8   // =1 
#define APP_event_IO_wakeup_sleep_BIT BIT9  // =1 io 休眠使能
#define APP_event_io_sleep_timer_BIT BIT10 // =1 启动io_sleep_timers定时器

#define APP_event_SP_flags_BIT BIT11 // =1 
#define APP_event_LP_flags_BIT BIT12 // =1
#define APP_event_LLP_flags_BIT BIT13 // =1

#define APP_event_IR_LED_flags_BIT BIT14 // =1 接收到IR数据正确
#define APP_event_BLE_CONNECTED_flags_BIT BIT15 // =1 接收到ble数据正确
#define APP_event_ds18b20_CONNECTED_flags_BIT BIT16 // =1 接收到ds18b20数据正确

#define APP_event_SNTP_ok_flags_BIT BIT17 //=1 时间同步完成。

#define APP_event_Log_msg_ok_flags_BIT BIT18 // =1 可以写数据

#define APP_event_Button_PRESS_REPEAT_BIT BIT19   // =1 按键三按以上
#define APP_event_Button_LONG_PRESS_BIT BIT20    // =1 按键长按上
#define APP_event_Button_wakeup_sleep_BIT BIT4    // =1 按键唤醒后进入休眠
#define APP_event_time_wakeup_sleep_BIT BIT5    // =1 时间唤醒后进入休眠

#define APP_event_LED_light_Always_off()  xEventGroupClearBits(APP_event_group,BIT0 | BIT1 | BIT2)//0 常灭
#define APP_event_LED_light_Breathe()  xEventGroupClearBits(APP_event_group,BIT1 | BIT2);xEventGroupSetBits(APP_event_group,BIT0) // 1 呼吸
#define APP_event_LED_light_100ms()  xEventGroupClearBits(APP_event_group,BIT0 | BIT2);xEventGroupSetBits(APP_event_group,BIT1) // 2 
#define APP_event_LED_light_500ms()  xEventGroupClearBits(APP_event_group,BIT2);xEventGroupSetBits(APP_event_group,BIT0 | BIT1) // 3 
#define APP_event_LED_light_1s()  xEventGroupClearBits(APP_event_group,BIT0 | BIT1);xEventGroupSetBits(APP_event_group,BIT2) // 4 
#define APP_event_LED_light_1s5()  xEventGroupClearBits(APP_event_group,BIT1);xEventGroupSetBits(APP_event_group,BIT0 | BIT2) // 5 
#define APP_event_LED_light_2s()  xEventGroupClearBits(APP_event_group,BIT0);xEventGroupSetBits(APP_event_group,BIT1 | BIT2) // 5
#define APP_event_LED_light_Always_On()  xEventGroupSetBits(APP_event_group,BIT0 | BIT1 | BIT2)//7 常亮

#define APP_event_LED_light_BIT 1
//最多23BIT

extern RTC_DATA_ATTR uint32_t sleep_keep;
#define sleep_keep_WIFI_AP_OR_STA_BIT BIT0
#define sleep_keep_Thermohygrometer_Low_battery_BIT  BIT1


#endif /* main_H */