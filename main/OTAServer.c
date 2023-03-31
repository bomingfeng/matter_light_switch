#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include "esp_ota_ops.h"
#include "freertos/event_groups.h"
#include "app_inclued.h"
#include <http_parser.h>
#include <esp_wifi.h>
#include "OTAServer.h"

//extern httpd_handle_t OTA_server = NULL;
int8_t flash_status = 0;
TimerHandle_t	xTimer_ota;
MessageBufferHandle_t HtmlToMcuData;
uint8_t page_feed = 1;
extern EventGroupHandle_t APP_event_group;


// Embedded Files. To add or remove make changes is component.mk file as well. 
extern const uint8_t Debug_parameters_html_start[] asm("_binary_Debug_parameters_html_start");
extern const uint8_t Debug_parameters_html_end[]   asm("_binary_Debug_parameters_html_end");


extern const uint8_t Multi_function_timing_html_start[] asm("_binary_Multi_function_timing_html_start");
extern const uint8_t Multi_function_timing_html_end[]   asm("_binary_Multi_function_timing_html_end");

//extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
//extern const uint8_t favicon_ico_end[]   asm("_binary_favicon_ico_end");
extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");

extern const uint8_t jquery_3_4_1_min_js_start[] asm("_binary_jquery_3_4_1_min_js_start");
extern const uint8_t jquery_3_4_1_min_js_end[]   asm("_binary_jquery_3_4_1_min_js_end");

RTC_DATA_ATTR  uint32_t Power_down_flag = 0;
RTC_DATA_ATTR  uint32_t TimeStatus = 85;//85 停止
RTC_DATA_ATTR  uint32_t AutolWateringTime = 50;
RTC_DATA_ATTR  uint32_t sleep_Time = 86400;//s 24H
RTC_DATA_ATTR  uint32_t deep_sleep_time = 0;
RTC_DATA_ATTR  uint32_t WateringFlag = 0;
//RTC_DATA_ATTR 不能使用uint64_t

static TimerHandle_t QuitTimers;

httpd_handle_t OTA_server = NULL;
uint32_t sse_id = 0x0,sse_data[sse_len] = {0};
/*****************************************************
 
	systemRebootTask()
 
	NOTES: This had to be a task because the web page needed
			an ack back. So i could not call this in the handler
 
 *****************************************************/
void systemRebootTask(void * parameter)
{
    uint64_t sleep_Time_us;
    EventBits_t staBits;
    xEventGroupClearBits(APP_event_group,APP_event_Button_wakeup_sleep_BIT | APP_event_time_wakeup_sleep_BIT);
    ESP_LOGI("otaserver","%d WateringFlag:%d:%p;Power_down_flag:%d:%p\n",__LINE__,WateringFlag,&WateringFlag,Power_down_flag,&Power_down_flag);
        
    xEventGroupWaitBits(APP_event_group,APP_event_Log_msg_ok_flags_BIT,pdTRUE,pdFALSE,portMAX_DELAY);
    
    if((WateringFlag == 0xaa) && (Power_down_flag == 0xaa)){
        WateringFlag = 0x55;
        deep_sleep_time = sleep_Time;
    }
    else{
        Power_down_flag = 0xaa;
        xEventGroupSetBits(APP_event_group,APP_event_Button_wakeup_sleep_BIT);
    }

	for (;;)
	{
        ESP_LOGI("otaserver","%d WateringFlag:%d:%p;Power_down_flag:%d:%p\n",__LINE__,WateringFlag,&WateringFlag,Power_down_flag,&Power_down_flag);
		// Wait here until the bit gets set for reboot
		staBits = xEventGroupWaitBits(APP_event_group, APP_event_log_spiffs_sleep_BIT | APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT, pdFALSE,	\
												pdTRUE, portMAX_DELAY);
		ESP_LOGI("otaserver","%d WateringFlag:%d:%p;Power_down_flag:%d:%p\n",__LINE__,WateringFlag,&WateringFlag,Power_down_flag,&Power_down_flag);
        if((staBits & (APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT)) == (APP_event_time_wakeup_sleep_BIT | APP_event_Button_wakeup_sleep_BIT)){
            if((deep_sleep_time <= 0x1FA4) && (AutolWateringTime != 0) && (TimeStatus ==170)){ //135min
                WateringFlag = 0xaa;
                sleep_Time_us = deep_sleep_time;
                sleep_Time_us *= 1000000;
            }
            else{
                sleep_Time_us = 0x1E2CC3100;
                deep_sleep_time -= 0x1FA4;
            }
          //  esp_wifi_stop();
           // esp_deep_sleep_set_rf_option(4);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            ESP_LOGI("OTA","Entering deep sleep!!!\r\n");
            esp_deep_sleep(sleep_Time_us);
		}
        else{

        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
	}		
}

/* Send index.html Page */
esp_err_t OTA_index_html_handler(httpd_req_t *req)
{
    xEventGroupSetBits(APP_event_group,APP_event_Force_off_lights_BIT);
    gpio_set_level(CONFIG_Lights_GPIO,0);
	ESP_LOGI("OTA", "index.html Requested");

	// Clear this every time page is requested 每次请求页面时清除此内容
	flash_status = 0;
	
	httpd_resp_set_type(req, "text/html");

	httpd_resp_send(req, (const char *)Multi_function_timing_html_start, Multi_function_timing_html_end - Multi_function_timing_html_start);

	return ESP_OK;
}

/* Send .ICO (icon) file  */
esp_err_t OTA_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI("OTA", "favicon_ico Requested");
    
	httpd_resp_set_type(req, "image/x-icon");

	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

/* jquery GET handler */
esp_err_t jquery_3_4_1_min_js_handler(httpd_req_t *req)
{
	ESP_LOGI("OTA", "jqueryMinJs Requested");
    //page_feed = 1;
	httpd_resp_set_type(req, "application/javascript");

	httpd_resp_send(req, (const char *)jquery_3_4_1_min_js_start, (jquery_3_4_1_min_js_end - jquery_3_4_1_min_js_start)-1);

	return ESP_OK;
}

/* Status */
esp_err_t OTA_update_status_handler(httpd_req_t *req)
{
	char ledJSON[300];
	
	ESP_LOGI("OTA", "Status Requested");

    /* Get WiFi Station configuration */
    wifi_config_t wifi_config;
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

	sprintf(ledJSON, "{\"status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\",\"Page_feed\":\"%d\",\"SysStatus\":\"%d\",\"ssid_text\":\"%s\",\"pass_text\":\"%s\",\"sleep_Time\":\"%d\",\"AutolWateringTime\":\"%d\",\"ADC_DATA\":\"%d\",\"Pincode\":\"%d\"}",  \
                       flash_status, __TIME__, __DATE__,page_feed,sse_data[0],wifi_config.sta.ssid, wifi_config.sta.password, sleep_Time, AutolWateringTime,sse_data[1],/*CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE*/20202021);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, ledJSON, strlen(ledJSON));
	
	// This gets set when upload is complete
	if (flash_status == 1)
	{
		// We cannot directly call reboot here because we need the 
		// browser to get the ack back. 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
	}
	return ESP_OK;
}
/* Receive .Bin file */
esp_err_t OTA_update_post_handler(httpd_req_t *req)
{
	esp_ota_handle_t ota_handle; 
	
	char ota_buff[1024];
	int content_length = req->content_len;
	int content_received = 0;
	int recv_len;
	bool is_req_body_started = false;
	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

	// Unsucessful Flashing 
	flash_status = -1;
	
	do
	{
		/* Read the data for the request */
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0) 
		{
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) 
			{
				ESP_LOGI("OTA", "Socket Timeout");
				/* Retry receiving if timeout occurred */
				continue;
			}
			ESP_LOGI("OTA", "OTA Other Error %d", recv_len);
			return ESP_FAIL;
		}

		ESP_LOGI("OTA","OTA RX: %d of %d\r", content_received, content_length);
		
	    // Is this the first data we are receiving 这是我们收到的第一个数据吗？
		// If so, it will have the information in the header we need. 如果是这样，它将在我们需要的标头中包含信息
		if (!is_req_body_started)
		{
			is_req_body_started = true;
			
			// Lets find out where the actual data staers after the header info		
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;	
			int body_part_len = recv_len - (body_start_p - ota_buff);
			
			int body_part_sta = recv_len - body_part_len;
			ESP_LOGI("OTA","OTA File Size: %d : Start Location:%d - End Location:%d\r\n", content_length, body_part_sta, body_part_len);
			ESP_LOGI("OTA","OTA File Size: %d\r\n", content_length);

			esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
			if (err != ESP_OK)
			{
				ESP_LOGI("OTA","Error With OTA Begin, Cancelling OTA\r\n");
				return ESP_FAIL;
			}
			else
			{
				ESP_LOGI("OTA","Writing to partition subtype %d at offset 0x%x\r\n", update_partition->subtype, update_partition->address);
			}

			// Lets write this first part of data out
			esp_ota_write(ota_handle, body_start_p, body_part_len);
		}
		else
		{
			// Write OTA data
			esp_ota_write(ota_handle, ota_buff, recv_len);
			
			content_received += recv_len;
		}
 
	} while (recv_len > 0 && content_received < content_length);

	// End response
	//httpd_resp_send_chunk(req, NULL, 0);

	
	if (esp_ota_end(ota_handle) == ESP_OK)
	{
		// Lets update the partition
		if(esp_ota_set_boot_partition(update_partition) == ESP_OK) 
		{
			const esp_partition_t *boot_partition = esp_ota_get_boot_partition();

			// Webpage will request status when complete 
			// This is to let it know it was successful
			flash_status = 1;
		
			ESP_LOGI("OTA", "Next boot partition subtype %d at offset 0x%x", boot_partition->subtype, boot_partition->address);
			ESP_LOGI("OTA", "Please Restart System...");
		}
		else
		{
			ESP_LOGI("OTA", "\r\n\r\n !!! Flashed Error !!!");
		}
		
	}
	else
	{
		ESP_LOGI("OTA", "\r\n\r\n !!! OTA End Error !!!");
	}
	
	return ESP_OK;

}


/*
httpd_uri_t OTA_index_html = {
	.uri = "/",
	.method = HTTP_GET,
	.handler = OTA_index_html_handler,
	// Let's pass response string in user
	//  context to demonstrate it's usage 
	.user_ctx = NULL
};
*/







esp_err_t HtmlToMcu_handler(httpd_req_t *req)
{
	char ota_buff[96];
	int content_length = req->content_len;
	int content_received = 0;
	int recv_len;
	//uint8_t i;
	do
	{
		/* Read the data for the request */
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0) 
		{
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) 
			{
				ESP_LOGI("otaserver","Socket Timeout");
				/* Retry receiving if timeout occurred */
				continue;
			}
			ESP_LOGI("otaserver","OTA Other Error %d", recv_len);
			return ESP_FAIL;
		}
		else
		{
			content_received += recv_len;
			//for(i = 0;i < content_length;i++)
//				printf("ota_buff:%s;\r\n",ota_buff);
			xMessageBufferSend(HtmlToMcuData,&ota_buff,content_length,portMAX_DELAY);
		}
	} while (recv_len > 0 && content_received < content_length);
	return ESP_OK;
}

    uint32_t test = 0x7FFFFFF;
esp_err_t McuToHtmlTest_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/event-stream;charset=utf-8");
    char ledJSON[96];
    sprintf(ledJSON, "id:%d\ndata:%d\n\n",12,test);
    httpd_resp_send(req, ledJSON, strlen(ledJSON));
    test--;
    return ESP_OK; 
}


esp_err_t McuToHtml_handler(httpd_req_t *req)
{

	httpd_resp_set_type(req, "text/event-stream;charset=utf-8");

	 /* \n是一个字符。buf_len长度要包函\n */
	//httpd_resp_send(req, "id:1\ndata:test\n\n",16);//以data: 开头会默认触发页面中message事件，以\n\n结尾结束一次推送。
	
	//httpd_resp_send(req, "id:1\nevent:foo\ndata:test\n\n",26);//'event:' + 事件名 + '\n'，这样就会触发页面中的foo事件而不是message事件，以\n\n结尾结束一次推送。


	/*转换*/
	char ledJSON[96];
	if(sse_id == 1){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 2){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 3){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 4){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 5){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 6){
        xTimerReset(QuitTimers,portMAX_DELAY);
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 7){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 8){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else if(sse_id == 9){
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	else{
        xTimerReset(QuitTimers,portMAX_DELAY);
		sse_id = 0;
		sprintf(ledJSON, "id:%d\ndata:%d\n\n",sse_id,sse_data[sse_id]);
	}
	
	httpd_resp_send(req, ledJSON, strlen(ledJSON));
	sse_id++;

	return ESP_OK;
}


/*
httpd_handle_t start_OTA_webserver(void)
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	
	// Lets change this from port 80 (default) to 8080
	//config.server_port = 8080;
	
	
	// Lets bump up the stack size (default was 4096)
	config.stack_size = 8192;
	
	
	// Start the httpd server
	ESP_LOGI("OTA", "Starting server on port: '%d'", config.server_port);
	
	if (httpd_start(&OTA_server, &config) == ESP_OK) 
	{
		// Set URI handlers
		ESP_LOGI("OTA", "Registering URI handlers");
		httpd_register_uri_handler(OTA_server, &OTA_index_html);
		//httpd_register_uri_handler(OTA_server, &OTA_favicon_ico);
		//httpd_register_uri_handler(OTA_server, &OTA_jquery_3_4_1_min_js);
		//httpd_register_uri_handler(OTA_server, &OTA_update);
		//httpd_register_uri_handler(OTA_server, &HtmlToMcu);
		//httpd_register_uri_handler(OTA_server, &McuToHtml);
		//httpd_register_uri_handler(OTA_server, &OTA_status);
		return OTA_server;
	}

	ESP_LOGI("OTA", "Error starting server!");
	return NULL;
}*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

static const char *TAG = "file_server";

/* Handler to redirect incoming GET request for /index.html to /
 * This can be overridden by uploading file with same name 
 用于重定向/索引的传入GET请求的处理程序。通过上传具有相同名称的文件，可以覆盖html*/
static esp_err_t index_html_get_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "307 Temporary Redirect");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}

/* Handler to respond with an icon file embedded in flash.
 * Browsers expect to GET website icon at URI /favicon.ico.
 * This can be overridden by uploading file with same name 
 * 处理程序以嵌入flash的图标文件进行响应。浏览器希望在URI/favicon.ico处获得网站图标。这可以通过上传同名文件来覆盖
*/
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
    const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
    return ESP_OK;
}


/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories 
  使用运行时生成的html发送HTTP响应，该html包含请求路径下的所有文件和文件夹的列表。
  对于SPIFFS，当路径为“/”以外的任何字符串时，返回空列表，因为SPIFFS不支持目录*/
static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath)
{
    char entrypath[FILE_PATH_MAX];
    char entrysize[16];
    const char *entrytype;

    struct dirent *entry;
    struct stat entry_stat;

    DIR *dir = opendir(dirpath);
    const size_t dirpath_len = strlen(dirpath);

    /* Retrieve the base path of file storage to construct the full path */
    strlcpy(entrypath, dirpath, sizeof(entrypath));

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
        /* Respond with 404 Not Found */
        //httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
       httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Send HTML file header 
    httpd_resp_sendstr_chunk(req,   \
    "<!DOCTYPE html>    \
    <html lang='zh-cmn-Hans'>   \
    <head>  \
        <meta charset='UTF-8'>  \
        <title>ESP32 OTA</title>    \
        <meta name='viewport' content='width=device-width, initial-scale=1'>  \
        <meta http-equiv='refresh' content='300'>   \
        <script src='jquery-3.4.1.min.js'></script>	\
    </head> \
    <body onload='getstatus();'>");

    // Get handle to embedded file upload script 
    const size_t index_html_size = (Debug_parameters_html_end - Debug_parameters_html_start);
    // Add file upload form and script which on execution sends a POST request to /upload 
    httpd_resp_send_chunk(req, (const char *)Debug_parameters_html_start, index_html_size);



    /* Get handle to embedded file upload script */
    extern const unsigned char upload_script_start[] asm("_binary_upload_script_html_start");
    extern const unsigned char upload_script_end[]   asm("_binary_upload_script_html_end");
    const size_t upload_script_size = (upload_script_end - upload_script_start);

    /* Add file upload form and script which on execution sends a POST request to /upload */
    httpd_resp_send_chunk(req, (const char *)upload_script_start, upload_script_size);

    /* Send file-list table definition and column labels */
    httpd_resp_sendstr_chunk(req,
        "<table class=\"fixed\" border=\"1\">"
        "<col width=\"800px\" /><col width=\"300px\" /><col width=\"300px\" /><col width=\"100px\" />"
        "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th><th>Delete</th></tr></thead>"
        "<tbody>");

    /* Iterate over all files / folders and fetch their names and sizes */
    while ((entry = readdir(dir)) != NULL) {
        entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        if (stat(entrypath, &entry_stat) == -1) {
            ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
            continue;
        }
        sprintf(entrysize, "%ld", entry_stat.st_size);
        ESP_LOGI(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize);


#if 1
        //<input type="button" value="Browse..." onclick="document.getElementById('selectedFile').click();" />
        /* Send chunk of HTML file containing table entries with file name and size */
        httpd_resp_sendstr_chunk(req, "<tr><td><input type='button' value='");
        //httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        if (entry->d_type == DT_DIR) {
            httpd_resp_sendstr_chunk(req, "/");
            httpd_resp_sendstr_chunk(req, entry->d_name);
        }
        httpd_resp_sendstr_chunk(req, "'style='border:none;outline:none;background:none;' onclick='download(value)'");
        httpd_resp_sendstr_chunk(req, "/></td><td>");

#elif
        /* Send chunk of HTML file containing table entries with file name and size */
        httpd_resp_sendstr_chunk(req, "<tr><td><a href=\"");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        if (entry->d_type == DT_DIR) {
            httpd_resp_sendstr_chunk(req, "/");
        }
        httpd_resp_sendstr_chunk(req, "\" target='_blank'>");
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "</a></td><td>");

#endif
        httpd_resp_sendstr_chunk(req, entrytype);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, entrysize);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/delete");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "\"><button type=\"submit\">Delete</button></form>");
        httpd_resp_sendstr_chunk(req, "</td></tr>\n");
    }
    closedir(dir);

    /* Finish the file list table */
    httpd_resp_sendstr_chunk(req, "</tbody></table>");

    httpd_resp_sendstr_chunk(req, "<hr color='red'/>");
     
    httpd_resp_sendstr_chunk(req, "<input type='button' id='Page_feed' value='主 页' style='margin-left:30px;'>");    
    httpd_resp_sendstr_chunk(req, "<input type='button' id='exit_set' value='退出设置' style='margin-left:30px;'>");      

    /* Send remaining chunk of HTML file to complete it */
    httpd_resp_sendstr_chunk(req, "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        ESP_LOGI(TAG, "File already exists:.pdf");
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        ESP_LOGI(TAG, "File already exists:.html");
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        ESP_LOGI(TAG, "File already exists:.jpeg");
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        ESP_LOGI(TAG, "File already exists:.ico");
        return httpd_resp_set_type(req, "image/x-icon");
    }
    ESP_LOGI(TAG, "File already exists:text/plain");
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) 
 将完整路径复制到目标缓冲区，并返回指向路径的指针（跳过前面的基本路径）*/
static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

static void vQuitTimersCallback(TimerHandle_t xTimer)
{
    xTimerStop(QuitTimers,portMAX_DELAY);
    xEventGroupClearBits(APP_event_group,APP_event_Force_off_lights_BIT);
}

/* Handler to download a file kept on the server */
static esp_err_t download_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    //FILE *fd = NULL;
    //struct stat file_stat;

    xEventGroupSetBits(APP_event_group,APP_event_Force_off_lights_BIT);
    gpio_set_level(CONFIG_Lights_GPIO,0);

    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
   
    //download_get: filename:/;filepath:/spiffs/;user_ctx:/spiffs;base_path:/spiffs;uri:/
    //E (12876) TEST: filename:/;filepath:/spiffs/;user_ctx:/spiffs;base_path:/spiffs;uri:/
    //I (12877) file_server: == '/'
    //I (12913) file_server: Found file : partition-table.bin (3072 bytes)
    //I (12917) file_server: Found file : hello.txt (12 bytes)
    //I (12952) OTA: jqueryMinJs Requested
   // ESP_LOGI("download_get", "filename:%s;filepath:%s;user_ctx:%s;base_path:%s;uri:%s",filename,filepath,req->user_ctx,((struct file_server_data *)req->user_ctx)->base_path,req->uri);

    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error 响应500内部服务器错误*/
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents 如果名称后面有“/”，则使用目录内容进行响应*/
    if (filename[strlen(filename) - 1] == '/') {
		ESP_LOGI(TAG, "== '/'");
        return http_resp_dir_html(req, filepath);
    }

#if 0
    if (stat(filepath, &file_stat) == -1) {
        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths 
         如果SPIFFS上不存在文件，请检查URI是否对应于硬编码路径之一*/
        if (strcmp(filename, "/index.html") == 0) {
            return index_html_get_handler(req);
        } else if (strcmp(filename, "/favicon.ico") == 0) {
            return favicon_get_handler(req);
        }
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               return ESP_FAIL;
           }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
//#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
//#endif
//    httpd_resp_send_chunk(req, NULL, 0);
#endif
    return ESP_OK;
}

/* Handler to upload a file onto the server */
static esp_err_t upload_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    /* Skip leading "/upload" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    //ESP_LOGI("upload", "filename:%s;filepath:%s;user_ctx:%s;base_path:%s;uri:%s",filename,filepath,req->user_ctx,((struct file_server_data *)req->user_ctx)->base_path,req->uri);
    // upload: filename:/bootloader.bin;filepath:/spiffs/bootloader.bin;user_ctx:/spiffs;base_path:/spiffs;uri:/upload/bootloader.bin
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == 0) {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        //httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        //httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
          //                  "File size must be less than "
        //                    MAX_FILE_SIZE_STR "!");
   //     httpd_resp_send_400(req);
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((struct file_server_data *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0) {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "buf:%s;", buf);
        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd))) {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File write failed!");
            /* Respond with 500 Internal Server Error */
            //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
//#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
//    httpd_resp_set_hdr(req, "Connection", "close");
//#endif
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

/* Handler to delete a file from the server */
static esp_err_t delete_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;

    /* Skip leading "/delete" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                            req->uri  + sizeof("/delete") - 1, sizeof(filepath));
   // ESP_LOGI("delete", "filename:%s;filepath:%s;user_ctx:%s;base_path:%s;uri:%s",filename,filepath,req->user_ctx,((struct file_server_data *)req->user_ctx)->base_path,req->uri);
    //filename:/hello.txt;filepath:/spiffs/hello.txt;user_ctx:/spiffs;base_path:/spiffs;uri:/delete/hello.txt
    //delete: filename:/test/hello.txt;filepath:/spiffs/test/hello.txt;user_ctx:/spiffs;base_path:/spiffs;uri:/delete/test/hello.txt
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "File does not exist : %s", filename);
        /* Respond with 400 Bad Request */
        //httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
     //   httpd_resp_send_400(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deleting file : %s", filename);
    /* Delete file */
    unlink(filepath);

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
//#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
//    httpd_resp_set_hdr(req, "Connection", "close");
//#endif
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

esp_err_t download_post_handler(httpd_req_t *req)
{
    FILE *fd = NULL;
    char filepath[FILE_PATH_MAX + 9] = "/spiffs";
    struct stat file_stat;
    char *filename = NULL;

    filename = calloc(1,FILE_PATH_MAX + 9);
    uint8_t i;

    for(i = 0;i < (FILE_PATH_MAX + 9);i++){
        *(filename + i) = *((req->uri) + 9 + i);
    }
    //TEST: uri:/spiffs/partition-table.bin;
    ESP_LOGI("TEST","filename:%s;",filename);

    for(i = 7;i < (FILE_PATH_MAX + 9);i++){
        filepath[i] = *((req->uri) + 2 + i);
    }    
    ESP_LOGI("TEST","filepath:%s;",filepath);


    //const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
    //                                         req->uri, sizeof(filepath));
    //filename:/download;filepath:/spiffs/download;user_ctx:/spiffs;base_path:/spiffs;uri:/download
    //filename:/download/partition-table.bin;filepath:/spiffs/download/partition-table.bin;user_ctx:/spiffs;base_path:/spiffs;uri:/download/partition-table.bin;
    //ESP_LOGI("TEST", "filename:%s;filepath:%s;user_ctx:%s;base_path:%s;uri:%s;",filename,filepath,req->user_ctx,((struct file_server_data *)req->user_ctx)->base_path,req->uri);
    
    //TEST: uri:/download/partition-table.bin;
    //ESP_LOGI("TEST","uri:%s;",req->uri);


    //delete: filename:/hello.txt;filepath:/spiffs/hello.txt;user_ctx:/spiffs;base_path:/spiffs;uri:/delete/hello.txt
    //delete: filename:/test/hello.txt;filepath:/spiffs/test/hello.txt;user_ctx:/spiffs;base_path:/spiffs;uri:/delete/test/hello.txt
    // upload: filename:/bootloader.bin;filepath:/spiffs/bootloader.bin;user_ctx:/spiffs;base_path:/spiffs;uri:/upload/bootloader.bin
   //download_get: filename:/;filepath:/spiffs/;user_ctx:/spiffs;base_path:/spiffs;uri:/

   //I (74802) download_get: filename:/favicon.ico;filepath:/spiffs/favicon.ico;user_ctx:/spiffs;base_path:/spiffs;uri:/favicon.ico
    //I (81802) download_get: filename:/favicon1.ico;filepath:/spiffs/favicon1.ico;user_ctx:/spiffs;base_path:/spiffs;uri:/favicon1.ico
#if 1
    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error 响应500内部服务器错误*/
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents 如果名称后面有“/”，则使用目录内容进行响应*/
    if (filename[strlen(filename) - 1] == '/') {
		ESP_LOGI(TAG, "== '/'");
        return http_resp_dir_html(req, filepath);
    }

    if (stat(filepath, &file_stat) == -1) {
        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths 
         如果SPIFFS上不存在文件，请检查URI是否对应于硬编码路径之一*/
        if (strcmp(filename, "/index.html") == 0) {
            return index_html_get_handler(req);
        } else if (strcmp(filename, "/favicon.ico") == 0) {
            return favicon_get_handler(req);
        }
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        //httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    ESP_LOGI(TAG, "chunk:%s;scratch:%s;", chunk,((struct file_server_data *)req->user_ctx)->scratch);
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               httpd_resp_send_500(req);
               return ESP_FAIL;
           }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
//#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
//    httpd_resp_set_hdr(req, "Connection", "close");
//#endif
    httpd_resp_send_chunk(req, NULL, 0);


#else
    fd = fopen(filename, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filename);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");//无法读取现有文件
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file :%s",filename);
    set_content_type_from_file(req, filename);

	//httpd_resp_set_type(req, "text/html");

	char buffer[1024];
	fgets(buffer, 1024, fd);
    httpd_resp_send(req, (const char *)buffer, strlen(buffer));

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
//#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
//    httpd_resp_set_hdr(req, "Connection", "close");
//#endif
    httpd_resp_send_chunk(req, NULL, 0);
#endif
    free(filename);
    return ESP_OK;
}

esp_err_t McuToHtmlData_post_handler(httpd_req_t *req)
{
    char ledJSON[100];
    uint8_t i;
// /McuToHtmlData/1
    for(i = 0;i < (FILE_PATH_MAX + 15);i++){
        ledJSON[i] = *((req->uri) + 15 + i);
    }
    //TEST: uri:/spiffs/partition-table.bin;
    ESP_LOGI("TEST","filename:%s;",ledJSON);
	
	sprintf(ledJSON, "{\"status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", flash_status, __TIME__, __DATE__);
	httpd_resp_set_type(req, "application/json;charset=utf-8");
	httpd_resp_send(req, ledJSON, strlen(ledJSON));

    return ESP_OK; 
}


httpd_handle_t server = NULL;
/* Function to start the file server */
esp_err_t start_file_server(const char *base_path)
{
    static struct file_server_data *server_data = NULL;

    QuitTimers = xTimerCreate("QuitTimers",180000/portTICK_PERIOD_MS,pdFALSE,( void * ) 3,vQuitTimersCallback);

    /* Validate file storage base path */
    if (!base_path || strcmp(base_path, "/spiffs") != 0) {
        ESP_LOGE(TAG, "File server presently supports only '/spiffs' as base path");//文件服务器目前仅支持“/spiffs”作为基本路径
        return ESP_ERR_INVALID_ARG;
    }

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");//文件服务器已启动
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");//未能为服务器数据分配内存
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme
     * 使用 URI 通配符匹配函数，以允许同一处理程序响应与通配符方案匹配的多个不同目标 URI
    */
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers=13;
    config.stack_size=12288;
    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }


    httpd_uri_t OTA_index_html = {
    .uri = "/index",
    .method = HTTP_GET,
    .handler = OTA_index_html_handler,
    // Let's pass response string in user
    //    * context to demonstrate it's usage 
    .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &OTA_index_html);


    /* URI handler for getting uploaded files */
    httpd_uri_t file_download = {
        .uri       = "/",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = download_get_handler,
        .user_ctx  = server_data    // Pass server data as context //给客户端的响应
    };
    httpd_register_uri_handler(server, &file_download);

    httpd_uri_t OTA_jquery_3_4_1_min_js = {
        .uri = "/jquery-3.4.1.min.js",
        .method = HTTP_GET,
        .handler = jquery_3_4_1_min_js_handler,
        /* Let's pass response string in user
        * context to demonstrate it's usage */
        .user_ctx = NULL //给客户端的响应
    };
	httpd_register_uri_handler(server, &OTA_jquery_3_4_1_min_js);

    httpd_uri_t OTA_favicon_ico = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = OTA_favicon_ico_handler,
        /* Let's pass response string in user
        * context to demonstrate it's usage */
        .user_ctx = NULL //给客户端的响应
    };
	httpd_register_uri_handler(server, &OTA_favicon_ico);

    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri       = "/upload/*",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = server_data    // Pass server data as context //给客户端的响应
    };
    httpd_register_uri_handler(server, &file_upload);

    /* URI handler for deleting files from server */
    httpd_uri_t file_delete = {
        .uri       = "/delete/*",   // Match all URIs of type /delete/path/to/file
        .method    = HTTP_POST,
        .handler   = delete_post_handler,
        .user_ctx  = server_data    // Pass server data as context //给客户端的响应
    };
    httpd_register_uri_handler(server, &file_delete);



    httpd_uri_t file_down = {
        .uri = "/download/*",
        .method = HTTP_POST,
        .handler = download_post_handler,
        .user_ctx = server_data //给客户端的响应
    };
    httpd_register_uri_handler(server, &file_down);

    httpd_uri_t OTA_status = {
        .uri = "/status",
        .method = HTTP_POST,
        .handler = OTA_update_status_handler,
        .user_ctx = NULL //给客户端的响应
    };

    httpd_register_uri_handler(server, &OTA_status);

    httpd_uri_t McuToHtml = {
        .uri = "/McuToHtml",
        .method = HTTP_GET,
        .handler = McuToHtml_handler,
        .user_ctx = NULL //给客户端的响应
    };
    httpd_register_uri_handler(server, &McuToHtml);

    httpd_uri_t OTA_update = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = OTA_update_post_handler,
        .user_ctx = NULL //给客户端的响应
    };
    httpd_register_uri_handler(server, &OTA_update);

    httpd_uri_t HtmlToMcu = {
        .uri = "/HtmlToMcu",
        .method = HTTP_POST,
        .handler = HtmlToMcu_handler,
        .user_ctx = NULL //给客户端的响应
    };
    httpd_register_uri_handler(server, &HtmlToMcu);

    httpd_uri_t McuToHtmlTest = {
        .uri = "/McuToHtmlTest",
        .method = HTTP_GET,
        .handler = McuToHtmlTest_handler,
        .user_ctx = NULL //给客户端的响应
    };
    httpd_register_uri_handler(server, &McuToHtmlTest);
	
    httpd_uri_t McuToHtmlData = {
        .uri = "/McuToHtmlData/*",
        .method = HTTP_POST,
        .handler = McuToHtmlData_post_handler,
        .user_ctx = NULL //给客户端的响应
    };
    httpd_register_uri_handler(server, &McuToHtmlData);	

    return ESP_OK;
}

/* Function to start the file server */
void stop_file_server(void)
{
	// Stop the httpd server
	httpd_stop(server);
}
