#include "htmltomcu.h"
#include <esp_wifi.h>

extern nvs_handle_t my_handle;
extern MessageBufferHandle_t HtmlToMcuData;
extern uint32_t sse_data[sse_len];
extern uint8_t page_feed;
extern EventGroupHandle_t APP_event_group;
RTC_DATA_ATTR extern uint32_t AutolWateringTime;
RTC_DATA_ATTR extern uint32_t sleep_Time;
RTC_DATA_ATTR extern uint32_t TimeStatus;//85 停止

void htmltomcudata_task(void * arg)
{
    char data[96];
    uint8_t i,data_ok,data_len,first_bit = 0,second_bit = 1,first_len,second_len;

    char ssid[32];      
    char password[64];

    uint32_t Pairing;

    esp_err_t err;

    uint32_t TimedOffHour = 0;
    uint32_t TimedOffmin = 0;
    uint32_t TimedOffsec = 0;

    uint32_t TimedOnHour = 0;
    uint32_t TimedOnmin = 0;
    uint32_t TimedOnsec = 0;

    uint8_t temp = 0;

    while(1)
    {
        data_len = xMessageBufferReceive(HtmlToMcuData,&data,96,(60000 / portTICK_PERIOD_MS)/*min*/ * 1);
        if(data_len > 0){
            ESP_LOGI("htmltomcu","data_len:%d;data:%s;\r\n",data_len,data);
            /*for(i = 0;i < data_len;i++){
                ESP_LOGI("htmltomcu","data[%d] = 0x%x;",i,data[i]);
            }
            ESP_LOGI("htmltomcu","\r\n");*/
            
            data_ok = 0;
            for(i = 0;i < 80;i++){
                if(('s' == data[i]) && ('s' == data[i+1]) && ('i' == data[i+2]) && ('d' == data[i+3]) && (':' == data[i+4])){
                    first_bit = i;
                    data_ok = 1;
                }
                if((data_ok == 1) && ('p' == data[i]) && ('a' == data[i+1]) && ('s' == data[i+2]) && ('s' == data[i+3]) && (':' == data[i+4])){
                    second_bit = i;
                    data_ok = 2;
                    break;
                }

                //TimedOffHour:12
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('f' == data[i+6]) && ('f' == data[i+7])    \
                    && ('H' == data[i+8]) && ('o' == data[i+9]) && ('u' == data[i+10]) && ('r' == data[i+11]) && (':' == data[i+12])){
                    first_bit = i;
                    data_ok = 3;
                    break;
                }       
                //TimedOffmin:11
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('f' == data[i+6]) && ('f' == data[i+7])    \
                    && ('m' == data[i+8]) && ('i' == data[i+9]) && ('n' == data[i+10]) && (':' == data[i+11])){
                    first_bit = i;
                    data_ok = 4;
                    break;
                }  
                //TimedOffsec:11
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('f' == data[i+6]) && ('f' == data[i+7])    \
                    && ('s' == data[i+8]) && ('e' == data[i+9]) && ('c' == data[i+10]) && (':' == data[i+11])){
                    first_bit = i;
                    data_ok = 5;
                    break;
                }   
                
                //TimedOnHour:11
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('n' == data[i+6]) && ('H' == data[i+7])    \
                    && ('o' == data[i+8]) && ('u' == data[i+9]) && ('r' == data[i+10]) && (':' == data[i+11])){
                    first_bit = i;
                    data_ok = 6;
                    break;
                } 
                //TimedOnmin:10
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('n' == data[i+6]) && ('m' == data[i+7])    \
                    && ('i' == data[i+8]) && ('n' == data[i+9]) && (':' == data[i+10])){
                    first_bit = i;
                    data_ok = 7;
                    break;
                } 
                //TimedOnsec:10
                if(('T' == data[i]) && ('i' == data[i+1]) && ('m' == data[i+2]) && ('e' == data[i+3])   \
                    && ('d' == data[i+4]) && ('O' == data[i+5]) && ('n' == data[i+6]) && ('s' == data[i+7])    \
                    && ('e' == data[i+8]) && ('c' == data[i+9]) && (':' == data[i+10])){
                    first_bit = i;
                    data_ok = 8;
                    break;
                }
                //exit_set:8
                if(('e' == data[i]) && ('x' == data[i+1]) && ('i' == data[i+2]) && ('t' == data[i+3])   \
                    && ('_' == data[i+4]) && ('s' == data[i+5]) && ('e' == data[i+6]) && ('t' == data[i+7])    \
                    && (':' == data[i+8])){
                    first_bit = i;
                    data_ok = 9;
                    break;
                }
                //page_feed:9
                if(('p' == data[i]) && ('a' == data[i+1]) && ('g' == data[i+2]) && ('e' == data[i+3])   \
                    && ('_' == data[i+4]) && ('f' == data[i+5]) && ('e' == data[i+6]) && ('e' == data[i+7])    \
                    && ('d' == data[i+8]) && (':' == data[i+9])){
                    first_bit = i;
                    data_ok = 10;
                    break;
                }

                //Pairing:7 11
                if(('P' == data[i]) && ('a' == data[i+1]) && ('i' == data[i+2]) && ('r' == data[i+3])   \
                    && ('i' == data[i+4]) && ('n' == data[i+5]) && ('g' == data[i+6]) && (':' == data[i+7])){
                    first_bit = i;
                    data_ok = 11;
                    break;
                }
            }
            if(data_ok == 2){
                //printf("first_bit:%d;second_bit:%d;\r\n",first_bit,second_bit);
                first_len = second_bit - (first_bit + 5);
                second_len = data_len - (second_bit + 5);
                //printf("first_len:%d;second_len:%d;\r\n",first_len,second_len);
                if((first_len > 0) && (second_len > 7)){
                    memset(ssid,'\0',sizeof(ssid));
                    for(i = 0;i < first_len;i++){
                        ssid[i] = data[first_bit + i + 5];
                    }

                    memset(password,'\0',sizeof(password));
                    for(i = 0;i < second_len;i++){
                        password[i] = data[second_bit + i + 5];
                    }
                    ESP_LOGI("htmltomcu", "ssid:(%s);password:(%s);\r\n",ssid,password);
                    /*
                    if(strcmp(&ssid,CONFIG_STATION_SSID) == 0){
                        printf("strcmp ssid OK\r\n");
                    }
                    if(strcmp(&password,CONFIG_STATION_PASSPHRASE) == 0){
                        printf("strcmp password OK\r\n");
                    }
                    */
                    
                    


                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    esp_wifi_init(&cfg);
                    esp_wifi_stop();
                    esp_wifi_set_mode(WIFI_MODE_STA);
                    wifi_config_t wifi_cfg = {0};
                    snprintf((char *)wifi_cfg.sta.ssid, sizeof(wifi_cfg.sta.ssid), "%s", ssid);
                    snprintf((char *)wifi_cfg.sta.password, sizeof(wifi_cfg.sta.password), "%s", password);
                    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
                    esp_wifi_start();
                    esp_wifi_connect();


                    xEventGroupSetBits(APP_event_group,APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT);
                }
            }
            else if(data_ok == 3){
                first_len = data_len - (first_bit + 13);
                ESP_LOGI("htmltomcu","first_len:%d;data_len:%d;first_bit:%d\r\n",first_len,data_len,first_bit);
                TimedOffHour = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 13] == ':'){//0:00
                        first_bit = i + 13;
                        break;
                    }
                    else if((data[first_bit + i + 13] >= '0') && (data[first_bit + i + 13] <= '9')){
                        TimedOffHour = 10 * TimedOffHour + (data[first_bit + i + 13] - '0');
                    }
                }
                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                TimedOffmin = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//01:01:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        TimedOffmin = 10 * TimedOffmin + (data[first_bit + i + 1] - '0');
                    }
                }
                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                TimedOffsec = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//0:0:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        TimedOffsec = 10 * TimedOffsec + (data[first_bit + i + 1] - '0');
                    }
                }

                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                TimedOnHour = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//0:0:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        TimedOnHour = 10 * TimedOnHour + (data[first_bit + i + 1] - '0');
                    }
                }
                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                TimedOnmin = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//0:0:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        TimedOnmin = 10 * TimedOnmin + (data[first_bit + i + 1] - '0');
                    }
                }
                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                TimedOnsec = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//0:0:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        TimedOnsec = 10 * TimedOnsec + (data[first_bit + i + 1] - '0');
                    }
                }
                
                ESP_LOGI("htmltomcu","first_bit:%d\r\n",first_bit);
                temp = 0;
                for(i = 0;i < first_len;i++){
                    if(data[first_bit + i + 1] == ':'){//0:0:0
                        first_bit += i + 1;
                        break;
                    }
                    else if((data[first_bit + i + 1] >= '0') && (data[first_bit + i + 1] <= '9')){
                        temp = 10 * temp + (data[first_bit + i + 1] - '0');
                    }
                }
                TimeStatus = temp;
                ESP_LOGI("htmltomcu","offHour:%d;offmin:%d;offsec:%d;onhour:%d;onmin:%d;onsec:%d;状态:%s:%d\r\n", \
                        TimedOffHour,TimedOffmin,TimedOffsec,TimedOnHour,TimedOnmin,TimedOnsec,TimeStatus>169?"运行":"停止",TimeStatus);
                sleep_Time = TimedOffHour * 3600 + TimedOffmin * 60 + TimedOffsec;
                AutolWateringTime = TimedOnHour * 3600 + TimedOnmin * 60 + TimedOnsec;
                ESP_LOGI("htmltomcu","sleep_Time:0x%xs;AutolWateringTime:%ds;\r\n",sleep_Time,AutolWateringTime);
            }
            else if(data_ok == 9){
                first_len = data_len - (first_bit + 9);
                ESP_LOGI("htmltomcu","first_len:%d;data_len:%d;first_bit:%d\r\n",first_len,data_len,first_bit);
                temp = 0;
                for(i = 0;i < first_len;i++){
                    if((data[first_bit + i + 9] >= '0') && (data[first_bit + i + 9] <= '9')){
                        temp = 10 * temp + (data[first_bit + i + 9] - '0');
                    }
                }
                if(temp >= 170){
                    xEventGroupSetBits(APP_event_group,APP_event_Button_PRESS_REPEAT_BIT);	
                    //xEventGroupSetBits(APP_event_group,APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT);
                }
            }  
            else if(data_ok == 10){
                first_len = data_len - (first_bit + 10);
                ESP_LOGI("htmltomcu","first_len:%d;data_len:%d;first_bit:%d\r\n",first_len,data_len,first_bit);
                temp = 0;
                for(i = 0;i < first_len;i++){
                    if((data[first_bit + i + 10] >= '0') && (data[first_bit + i + 10] <= '9')){
                        temp = 10 * temp + (data[first_bit + i + 10] - '0');
                    }
                }
                if(temp >= 170){
                    page_feed = 0;
                }
            }    
            else if(data_ok == 11){//Pairing:7 11
                first_len = data_len - (first_bit + 8);
                Pairing = 0;
                for(i = 0;i < first_len;i++){
                    if((data[first_bit + i + 8] >= '0') && (data[first_bit + i + 8] <= '9')){
                        Pairing = 10 * Pairing + (data[first_bit + i + 8] - '0');
                    }
                }
                ESP_LOGI("htmltomcu","data_len:%d;data_ok:%d;Node_ID:%d-0x%x",data_len,data_ok,Pairing,Pairing);

/*

                // Open
                ESP_LOGI("htmltomcu", "Opening Non-Volatile Storage (NVS) handle... ");
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK) {
                    ESP_LOGI("htmltomcu", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
                } else {
                    ESP_LOGI("htmltomcu", "Done\n");
                }   
                // Write
                ESP_LOGI("htmltomcu", "Updating restart counter in NVS ... \n");
                
                err=nvs_set_str(my_handle,"wifissid",ssid);
                if(err != ESP_OK){
                    ESP_LOGI("htmltomcu","Failed!\r\n");
                }
                else{
                    ESP_LOGI("htmltomcu","Done!\r\n");
                }

                err=nvs_set_str(my_handle,"wifipass",password);
                if(err != ESP_OK){
                    ESP_LOGI("htmltomcu","Failed!\r\n");
                }
                else{
                    ESP_LOGI("htmltomcu","Done!\r\n");
                }
                
                err=nvs_set_i8(my_handle,"wifi_mode",WIFI_MODE_STA);
                if(err != ESP_OK){
                    ESP_LOGI("htmltomcu","Failed!\r\n");
                }
                else{
                    ESP_LOGI("htmltomcu","Done!\r\n");
                }
                // Commit written value.
                // After setting any values, nvs_commit() must be called to ensure changes are written
                // to flash storage. Implementations may write to storage at other times,
                // but this is not guaranteed.
                ESP_LOGI("htmltomcu", "Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                if(err != ESP_OK){
                    ESP_LOGI("htmltomcu","Failed!\r\n");
                }
                else{
                    ESP_LOGI("htmltomcu","Done!\r\n");
                }
                // Close
                nvs_close(my_handle);
                */

            } 
            else{

            }
            data_len = 0;
        }

    }
}
