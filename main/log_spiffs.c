#include "log_spiffs.h"
#include "esp_spiffs.h"
#include "OTAServer.h"

extern nvs_handle_t my_handle;

char dectohex[10] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
char log_msg[200];



static uint8_t spiffs_write_temperature(uint8_t groud,size_t *used,size_t full_used,char *log_msg,uint8_t deleteFlag)
{

    struct stat file_stat;
    char buffer[298];
    uint32_t SPIFFS_config = 0x04FFFFFF;
    esp_err_t ret;
    char filepath[FILE_PATH_MAX] = "/spiffs/temperature_log0.txt";
    
    if(deleteFlag == 0xaa){
        if(groud == 9){
            groud = 0;
        }
        else{
            groud++;
        }
        filepath[23] = dectohex[groud];
        unlink(filepath);
        *used = full_used;

        SPIFFS_config = (groud << 28) | 0x04000000 | full_used;
        // Open
        ESP_LOGI("app_main", "\n");
        ESP_LOGI("app_main", "Opening Non-Volatile Storage (NVS) handle... ");
        ret = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (ret != ESP_OK) 
        {
            ESP_LOGI("app_main", "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
        }
        else 
        {   
            ESP_LOGI("app_main", "Done\n");
            // Write
            ESP_LOGI("app_main", "Updating restart counter in NVS ... ");
            if(nvs_set_u32(my_handle, "SPIFFS_config", SPIFFS_config) != ESP_OK){
                ESP_LOGI("app_main","Failed!\n");
            }
            else{
                    ESP_LOGI("app_main","Done\n");
            }
            

            // Commit written value.
            // After setting any values, nvs_commit() must be called to ensure changes are written
            // to flash storage. Implementations may write to storage at other times,
            // but this is not guaranteed.
            ESP_LOGI("app_main","Committing updates in NVS ... ");
            ret = nvs_commit(my_handle);
            //printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");
        }
        // Close
        nvs_close(my_handle);
    }
    else{
        filepath[23] = dectohex[groud];
    }
    
    ESP_LOGI("temperature_log", "1Opening file temperature_log%d:%s",groud,filepath);
    FILE* f = fopen(filepath, "a");
    ESP_LOGI("temperature_log", "2Opening file temperature_log%d:%s",groud,filepath);
    if (f == NULL) {
        ESP_LOGE("app_main", "Failed to open file for writing");
    }
    else{
        stat(filepath, &file_stat);
        
        ESP_LOGI("temperature_log", "file_stat.st_size%ld;used:%d",file_stat.st_size,*used);
        if((file_stat.st_size + 128) >= full_used){
            if(groud == 9){
                groud = 0;
            }
            else{
                groud++;
            }
            fclose(f);
            filepath[23] = dectohex[groud];
            unlink(filepath);
            ESP_LOGI("temperature_log", "3Opening file temperature_log%d:%s",groud,filepath);
            FILE* f = fopen(filepath, "a");
            if (f == NULL) {
                ESP_LOGE("app_main", "Failed to open file for writing");
            }
            *used = full_used;
            SPIFFS_config = (groud << 28) | 0x04000000 | full_used;
            // Open
            ESP_LOGI("app_main", "\n");
            ESP_LOGI("app_main", "Opening Non-Volatile Storage (NVS) handle... ");
            ret = nvs_open("storage", NVS_READWRITE, &my_handle);
            if (ret != ESP_OK) 
            {
                ESP_LOGI("app_main", "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
            }
            else 
            {   
                ESP_LOGI("app_main", "Done\n");
                // Write
                ESP_LOGI("app_main", "Updating restart counter in NVS ... ");
                if(nvs_set_u32(my_handle, "SPIFFS_config", SPIFFS_config) != ESP_OK){
                    ESP_LOGI("app_main","Failed!\n");
                }
                else{
                        ESP_LOGI("app_main","Done\n");
                }
                

                // Commit written value.
                // After setting any values, nvs_commit() must be called to ensure changes are written
                // to flash storage. Implementations may write to storage at other times,
                // but this is not guaranteed.
                ESP_LOGI("app_main","Committing updates in NVS ... ");
                ret = nvs_commit(my_handle);
                //printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");
            }
            // Close
            nvs_close(my_handle);
        }
        else{
            *used = full_used - file_stat.st_size;
            ESP_LOGI("app_main", "10");
        }
    }
    if(f != NULL )
    {
        sprintf(buffer,"%s \r",log_msg);//2018-09-19 08:59:07
        //fprintf(f, '%s',buffer);//2018-09-19 08:59:07
        fwrite(buffer, strlen(buffer), 1, f);
        *used -= strlen(buffer);
        ESP_LOGI("app_main", "File written");
        fclose(f);
    }
    return groud;
}
/*                xEventGroupWaitBits(APP_event_group,APP_event_Log_msg_ok_flags_BIT,pdTRUE,pdFALSE,portMAX_DELAY);
                sprintf(log_msg,"小时温度记录,蓝牙温度:%d,蓝牙最高温度:%d,蓝牙最低温度:%d;DS18B20:%d,DS18最高温度:%d,DS18最低温度:%d。",    \
                bleC,bleMax,bleMin,ds18b20C,ds18Max,ds18Min);
*/                
void log_task(void * arg)
{
    size_t total = 0, used = 0;
    esp_err_t ret;
    uint8_t Run_once = 0,useding = 0,deleteFlag = 0x55;

    uint32_t SPIFFS_config = 0x04FFFFFF;
    //uint32_t log_temp;
    /*
    SPIFFS_config
    High                                                                                            Low
    0 0 0 0  0 0 0 0                                0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0   
    4位编号。 27bit文件不存在或已删除,26bit正在读写。  可使用容量
    0x04000000  
    0x18000000
    0x28000000
    0x38000000
    0x48000000
    ...
    0xF8000000
    */
 /*   xEventGroupWaitBits(APP_event_group, \
            APP_event_SNTP_ok_flags_BIT, \
            pdFALSE,                               \
            pdFALSE,                               \
            portMAX_DELAY);
*/
    xEventGroupSetBits(APP_event_group,APP_event_Log_msg_ok_flags_BIT);
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE("app_main", "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else{
        ESP_LOGI("app_main", "Partition size: total: %d, used: %d", total, used);

        ret = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (ret != ESP_OK) 
        {
            ESP_LOGE("app_main", "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
        }
        else 
        {
            // Read
            ESP_LOGI("app_main", "Reading restart counter from NVS ... ");
            ret = nvs_get_u32(my_handle, "SPIFFS_config", &SPIFFS_config);
            switch (ret) {
                case ESP_OK:
                    ESP_LOGI("app_main", "SPIFFS_config = %d\n", SPIFFS_config);
                    if((SPIFFS_config & 0x04000000) != 0){
                        size_t used1;
                        useding = (SPIFFS_config & 0xF0000000) >> 28;
                        used1 = SPIFFS_config & 0x00FFFFFF;
                        if(used1 <= 128)
                        {
                            deleteFlag = 0xaa;
                            used = total/10; 
                        }
                        else{
                            used = used1;
                        }
                    }
                    else{
                        deleteFlag = 0xaa;
                        used = total/10; 
                    }
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGI("app_main", "The value is not initialized yet!\n");
                    deleteFlag = 0xaa;
                    used = total/10; 
                    useding = 9;
                    break;
                default :
                    ESP_LOGI("app_main", "Error (%s) reading!\n", esp_err_to_name(ret));
                    deleteFlag = 0xaa;
                    used = total/10; 
                    useding = 9;
                    break;
            }
            // Close
            nvs_close(my_handle);
        }
    }
    while(1){
        EventBits_t staBits = xEventGroupGetBits(APP_event_group);
        if(((staBits & APP_event_Log_msg_ok_flags_BIT) != APP_event_Log_msg_ok_flags_BIT) && (Run_once == 0)){
            vTaskDelay(500 / portTICK_PERIOD_MS);
            useding = spiffs_write_temperature(useding,&used,total/10,log_msg,deleteFlag);
            ESP_LOGI("app_main", "useding:%d;used:%d;total:%d;deleteFlag:%d;\n",useding,used,total/10,deleteFlag);
            deleteFlag = 0x55;
            xEventGroupSetBits(APP_event_group,APP_event_Log_msg_ok_flags_BIT);
        }
        else if((staBits & (APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT)) == (APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT)){
            if(Run_once == 0){
                Run_once = 1;
                SPIFFS_config = (useding << 28) | 0x04000000 | used;
                // Open
                ESP_LOGI("app_main", "\n");
                ESP_LOGI("app_main", "Opening Non-Volatile Storage (NVS) handle... ");
                ret = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (ret != ESP_OK) 
                {
                    ESP_LOGI("app_main", "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
                }
                else 
                {   
                    ESP_LOGI("app_main", "Done\n");
                    // Write
                    ESP_LOGI("app_main", "Updating restart counter in NVS ... ");
                    if(nvs_set_u32(my_handle, "SPIFFS_config", SPIFFS_config) != ESP_OK){
                        ESP_LOGI("app_main","Failed!\n");
                    }
                    else{
                            ESP_LOGI("app_main","Done\n");
                    }
                    

                    // Commit written value.
                    // After setting any values, nvs_commit() must be called to ensure changes are written
                    // to flash storage. Implementations may write to storage at other times,
                    // but this is not guaranteed.
                    ESP_LOGI("app_main","Committing updates in NVS ... ");
                    ret = nvs_commit(my_handle);
                    //printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");
                }
                // Close
                nvs_close(my_handle);
                xEventGroupSetBits(APP_event_group,APP_event_log_spiffs_sleep_BIT);
            }
        }
        else{
            Run_once = 0;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/* Function to initialize SPIFFS */
esp_err_t init_spiffs(void)
{
    ESP_LOGI("app_main", "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,   // This decides the maximum number of files that can be created on the storage
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("app_main", "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("app_main", "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE("app_main", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE("app_main", "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGI("app_main", "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}
