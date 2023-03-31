#include "myWiFi.h"
#include <esp_wifi.h>
#include "OTAServer.h"


//#define WIFI_MODE_AP 0xaa
//#define WIFI_MODE_STA 0x55

extern EventGroupHandle_t APP_event_group;

static nvs_handle_t my_handle;
static int webserver_status = 0x55;

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	esp_err_t err;

    switch (event_id) {
		case IP_EVENT_STA_GOT_IP: 
		{
 			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			/*ip_addr1 = esp_ip4_addr1(&event->ip_info.ip);	
			ip_addr2 = esp_ip4_addr2(&event->ip_info.ip);
			ip_addr3 = esp_ip4_addr3(&event->ip_info.ip);
			ip_addr4 = esp_ip4_addr4(&event->ip_info.ip);	*/	
        	ESP_LOGI("MyWiFi","%d行got ip:" IPSTR,__LINE__,IP2STR(&event->ip_info.ip));

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
			
			/*
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
			*/
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

			xEventGroupSetBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);	
			xEventGroupClearBits(APP_event_group, APP_event_WIFI_AP_CONNECTED_BIT);
			/* Start the web server */
			if(webserver_status == 0x55){
				//start_OTA_webserver();
				ESP_ERROR_CHECK(start_file_server("/spiffs"));
				webserver_status = 0xaa;
			}
			esp_restart();
			break;
		}
		case IP_EVENT_STA_LOST_IP: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_LOST_IP\r\n");
			break;
		}
		case IP_EVENT_GOT_IP6: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_GOT_IP6\r\n");
			break;
		}
		default: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_ OTHER\r\n");
			break;
		}	
    }
    return;
}


static int s_retry_num = 0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    switch (event_id) {
		case WIFI_EVENT_WIFI_READY: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_WIFI_READY\r\n");
			break;
		}
		case WIFI_EVENT_SCAN_DONE: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_SCAN_DONE\r\n");
			break;
		}
		case WIFI_EVENT_STA_START: {
			esp_wifi_connect();
			ESP_LOGI("MyWiFi","WIFI_EVENT_STA_START\r\n");
			break;
		}
		case WIFI_EVENT_STA_STOP: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_STOP\r\n");
			break;
		}
		case WIFI_EVENT_STA_CONNECTED: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_CONNECTED\r\n");
			break;
		}
		case WIFI_EVENT_STA_DISCONNECTED: {
			xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
			if (s_retry_num <= 100) 
			{
				esp_wifi_connect();
				s_retry_num++;
				//if((s_retry_num % 10) == 0){
				//	xTimerReset(QuitTimers,portMAX_DELAY);
				//}
				ESP_LOGI("MyWiFi","retry to connect to the AP.(%d / %d)\n", s_retry_num,100);
			} 
			else
			{
				ESP_LOGI("MyWiFi","connect to the AP fail\r\n");
				xEventGroupSetBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
				s_retry_num = 0;
			}		
			break;
		}
		case WIFI_EVENT_STA_AUTHMODE_CHANGE: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_AUTHMODE_CHANGE\r\n");
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_SUCCESS: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_WPS_ER_SUCCESS\r\n");
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_FAILED: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_WPS_ER_FAILED\r\n");
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_TIMEOUT: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT\r\n");
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_PIN: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_STA_WPS_ER_PIN\r\n");
			break;
		}
		case WIFI_EVENT_AP_START: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_AP_START\r\n");
			xEventGroupSetBits(APP_event_group, APP_event_WIFI_AP_CONNECTED_BIT);
			break;
		}
		case WIFI_EVENT_AP_STOP: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_AP_STOP\r\n");
			break;
		}
		case WIFI_EVENT_AP_STACONNECTED: {
			//xTimerStop(QuitTimers,portMAX_DELAY);
			//xTimerChangePeriod(QuitTimers,180000/ portTICK_PERIOD_MS,portMAX_DELAY);
			//xTimerReset(QuitTimers,portMAX_DELAY);
			wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
			/*ip_addr1 = 192;	
			ip_addr2 = 168;
			ip_addr3 = 0;
			ip_addr4 = 1;	*/
        	ESP_LOGI("MyWiFi","station "MACSTR" join, AID=%d\r\n",	\
               MAC2STR(event->mac), event->aid);
			xEventGroupSetBits(APP_event_group, APP_event_WIFI_AP_CONNECTED_BIT);
			xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
			/* Start the web server */
			if(webserver_status == 0x55){
				//start_OTA_webserver();
				ESP_ERROR_CHECK(start_file_server("/spiffs"));
				webserver_status = 0xaa;
			}
			break;
		}
		case WIFI_EVENT_AP_STADISCONNECTED: {
			wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        	ESP_LOGI("MyWiFi","station "MACSTR" leave, AID=%d\r\n",	\
                MAC2STR(event->mac), event->aid);
			xEventGroupClearBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
			break;
		}
		case WIFI_EVENT_AP_PROBEREQRECVED: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_AP_PROBEREQRECVED\r\n");
			break;
		}
		default: {
			ESP_LOGI("WiFI", "SYSTEM_EVENT_OTHER\r\n");
			//xEventGroupSetBits(APP_event_group, APP_event_WIFI_AP_CONNECTED_BIT);
			//xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
			//esp_wifi_disconnect();
			//esp_wifi_stop();
			break;
		}	
    }
    return;
}

/* -----------------------------------------------------------------------------
  start_dhcp_server(void)

  Notes:  
  
 -----------------------------------------------------------------------------*/
void start_dhcp_server(void) 
{
	tcpip_adapter_ip_info_t info;
	ESP_LOGI("test", "WIFI_MODE_AP12.");
	// initialize the tcp stack
	//ESP_ERROR_CHECK(esp_netif_init());
	//tcpip_adapter_init();
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info));

	ESP_LOGI("test", "WIFI_MODE_AP13.");
	// stop DHCP server
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	ESP_LOGI("test", "WIFI_MODE_AP14.");
	// assign a static IP to the network interface

    IP4_ADDR(&info.ip, 192, 168 , 88, 2);
    IP4_ADDR(&info.gw, 192, 168 , 88, 2);
    IP4_ADDR(&info.netmask, 255, 255 , 255, 0); 
	 
	ESP_LOGI("test", "WIFI_MODE_AP15.");
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
	
	ESP_LOGI("test", "WIFI_MODE_AP16.");
	// start the DHCP server   
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
	ESP_LOGI("WiFI","DHCP server started \n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void WIFI_Mode_Save(wifi_mode_t WifiMode)
{
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

wifi_mode_t WIFI_Mode_Check(void)
{
    wifi_mode_t enWifiMode = WIFI_MODE_MAX;
    int8_t u8ProvAvlFlag = 2;

    // Open
    //printf("\n");
    //printf("Opening Non-Volatile Storage (NVS) handle... ");
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
       //printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else 
    {
        //printf("Done\n");

        // Read
        //printf("Reading restart counter from NVS ... \n");
        
        err = nvs_get_i8(my_handle, "wifi_mode", &u8ProvAvlFlag);
        switch (err) 
        {
            case ESP_OK:
                //printf("Done\n");
				switch (u8ProvAvlFlag){
					case WIFI_MODE_NULL:
						enWifiMode = WIFI_MODE_NULL;
						enWifiMode = WIFI_MODE_AP;
					break;
					case WIFI_MODE_STA:
						enWifiMode = WIFI_MODE_STA;
					break;
					case WIFI_MODE_AP:
						enWifiMode = WIFI_MODE_AP;
					break;
					case WIFI_MODE_APSTA:
						enWifiMode = WIFI_MODE_APSTA;
					break;
					case WIFI_MODE_MAX:
						enWifiMode = WIFI_MODE_MAX;
					break;
					default :
						enWifiMode = WIFI_MODE_AP;
					break;
				}
                ESP_LOGI("WiFi","wifi_mode:%d\n",enWifiMode);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                //printf("The value is not initialized yet!\n");
				enWifiMode = WIFI_MODE_AP;
                break;
            default :
                //printf("Error (%s) reading!\n", esp_err_to_name(err));
				enWifiMode = WIFI_MODE_AP;
                break;
        }       
        // Close
        nvs_close(my_handle);
    }
    return enWifiMode;
}
/*************************************************
 * Function Implementation (External and Local)
 ************************************************/
static void WIFI_vInitCommon(void)
{
	// Set the default wifi logging
	//esp_log_level_set("wifi", ESP_LOG_ERROR);
    //esp_log_level_set("wifi", ESP_LOG_WARN);
    //esp_log_level_set("wifi", ESP_LOG_INFO);

	/* Initialize TCP/IP 
	 * Alternative for ESP_ERROR_CHECK(esp_netif_init()); 
	 */
	//ESP_ERROR_CHECK(esp_netif_init());
	tcpip_adapter_init();

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	/* Create the event group to handle wifi events */


    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

	/* Initialize Wi-Fi */
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
}



static void WIFI_vConfigSoftAP(void *arg)
{
	// configure the wifi connection and start the interface
	wifi_config_t ap_config = {
		.ap = {
			.ssid = CONFIG_AP_SSID,
		.password = CONFIG_AP_PASSPHARSE,
		.ssid_len = strlen(CONFIG_AP_SSID),
		.channel = 1,
		.authmode = WIFI_AUTH_WPA_WPA2_PSK,
		.ssid_hidden = 0,
		.max_connection = 1,
		.beacon_interval = 100		
		},
	};
    if (strlen(CONFIG_AP_PASSPHARSE) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }	

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));


	ESP_LOGI("test", "WIFI_MODE_AP11.");
	//printf("ESP WiFi started in AP mode \n");
	
	
	 
	//  Spin up a task to show who connected or disconected
	 
	 
	//xTaskCreate(&print_sta_info, "print_sta_info", 4096, NULL, 1, NULL);

	// https://demo-dijiudu.readthedocs.io/en/latest/api-reference/wifi/esp_wifi.html#_CPPv225esp_wifi_set_max_tx_power6int8_t
	// This can only be placed after esp_wifi_start();
	//ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(8));
	ESP_LOGI("test", "WIFI_MODE_AP12.");
}

wifi_config_t wifi_config = {
	.sta = {
		.ssid = CONFIG_STATION_SSID,
		.password = CONFIG_STATION_PASSPHRASE,

		/* Setting a password implies station will connect to all security modes including WEP/WPA.
		* However these modes are deprecated and not advisable to be used. Incase your Access point
		* doesn't support WPA2, these mode can be enabled by commenting below line */
		//.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		.pmf_cfg = {
			.capable = true,
			.required = false
		},
	},
};


static void WIFI_vConfigStation(void *arg)
{
	size_t len;
	char ssid[32];      
    char password[64];

	// Open
    //printf("\n");
    //printf("Opening Non-Volatile Storage (NVS) handle... ");

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
       //printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else 
    {
        //printf("Done\n");

        // Read
        //printf("Reading restart counter from NVS ... \n");
        len = sizeof(ssid);
        err = nvs_get_str(my_handle, "wifissid", ssid, &len);
        switch (err) 
        {
            case ESP_OK:
                //printf("Done\n");
				strcpy((char *)wifi_config.sta.ssid, ssid);
                ESP_LOGI("WiFi","wifissid:(%s)",wifi_config.sta.ssid);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                //printf("The value is not initialized yet!\n");
                break;
            default :
                //printf("Error (%s) reading!\n", esp_err_to_name(err));
                break;
        }
        len = sizeof(password);
        err = nvs_get_str(my_handle, "wifipass", password, &len);
        switch (err) 
        {
            case ESP_OK:
                //printf("Done\n");
				strcpy((char *)wifi_config.sta.password,password);
                ESP_LOGI("WiFi","wifipass:(%s)\n",wifi_config.sta.password);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                //printf("The value is not initialized yet!\n");
                break;
            default :
                //printf("Error (%s) reading!\n", esp_err_to_name(err));
                break;
        }
        // Close
        nvs_close(my_handle);
    }
	
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	
	ESP_LOGI("WiFi", "Station Set to SSID:(%s) Pass:(%s)\r\n", wifi_config.sta.ssid, wifi_config.sta.password);
}

#if 0
void vTaskWifiHandler(void * pvParameters)
{
    uint8_t WIFI_AP_count = 0;
    uint8_t enWifiMode = WIFI_MODE_STA;
    EventBits_t EventBit;

    vTaskDelay(60000 / portTICK_PERIOD_MS);
    while(1){
        EventBit = xEventGroupGetBits(APP_event_group);
        if((EventBit & APP_event_WIFI_AP_CONNECTED_BIT) == APP_event_WIFI_AP_CONNECTED_BIT){
            ESP_LOGI("MyWiFi","enWifiMode == WIFI_MODE_AP");
            if((enWifiMode != WIFI_MODE_AP) && (WIFI_AP_count >= 9)){
                stop_file_server();
                //esp_wifi_disconnect();
                esp_wifi_stop();


                wifi_config_t wifi_config = {
                    .ap = {
                        .ssid = "myssid",
                        .ssid_len = strlen("myssid"),
                        .channel = 1,
                        .password = "mypassword",
                        .max_connection = 4,
                        .authmode = WIFI_AUTH_WPA_WPA2_PSK
                    },
                };
                if (strlen("mypassword") == 0){
                    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
                }
                esp_wifi_set_mode(WIFI_MODE_AP);
                esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
                esp_wifi_start();
                ESP_LOGI("MyWiFi","Wifi SofAP Start!");

                //start_dhcp_server();
                enWifiMode = WIFI_MODE_AP;
                WIFI_AP_count = 0;
            }
            if(WIFI_AP_count >= 181){
                xEventGroupClearBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
                WIFI_AP_count = 0;
            }
            WIFI_AP_count++;
        }
        else{
            ESP_LOGI("MyWiFi","enWifiMode == WIFI_MODE_STA");
            WIFI_AP_count = 0;
            if((enWifiMode != WIFI_MODE_STA) && ((EventBit & APP_event_WIFI_STA_CONNECTED_BIT) != APP_event_WIFI_STA_CONNECTED_BIT)){

                stop_file_server();
                tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
                esp_wifi_stop();
                esp_wifi_set_mode(WIFI_MODE_STA);
                esp_wifi_start();
                esp_wifi_connect();
                ESP_LOGI("MyWiFi","Wifi Station Start!");

                enWifiMode = WIFI_MODE_STA;
            }
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
#endif

#if 1

void vTaskWifiHandler(void * pvParameters)
{
    wifi_mode_t enWifiMode;
	EventBits_t bits; 
	uint32_t stacount = 0;
	uint32_t apcount = 0;
	xEventGroupClearBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT | APP_event_WIFI_STA_CONNECTED_BIT);

	enWifiMode = WIFI_Mode_Check();
	
    /* Initializing Wi-Fi Station / AccessPoint*/
    WIFI_vInitCommon();
    switch(enWifiMode)
    {
		case WIFI_MODE_NULL:
		break;
		case WIFI_MODE_APSTA:
		break;
		case WIFI_MODE_MAX:
		break;

        case WIFI_MODE_STA:
			//sse_data[sse_Least] &= ~BIT4;
			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            WIFI_vConfigStation(&OTA_server);
			/* Start Wi-Fi Station / AccessPoint*/
			ESP_ERROR_CHECK(esp_wifi_start());
			bits =xEventGroupWaitBits(APP_event_group, APP_event_WIFI_STA_CONNECTED_BIT, pdFALSE, pdFALSE, (1000 / portTICK_PERIOD_MS)/*min*/ * 220);
			if((bits & APP_event_WIFI_STA_CONNECTED_BIT) != APP_event_WIFI_STA_CONNECTED_BIT){
				xEventGroupSetBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
				xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
			}
        break;

        case WIFI_MODE_AP:
			//sse_data[sse_Least] |= BIT4;
			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
            WIFI_vConfigSoftAP(&OTA_server);
			/* Start Wi-Fi Station / AccessPoint*/
			ESP_ERROR_CHECK(esp_wifi_start());
			start_dhcp_server();
        break;

        default:
        break;
    }
    for(;;)
	{
		if(enWifiMode == WIFI_MODE_STA)
		{
			//vTaskDelay(60000 / portTICK_PERIOD_MS);
			//xEventGroupSetBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
			//xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);


			// wait for connection or disconnect event
			bits = xEventGroupWaitBits(APP_event_group, APP_event_WIFI_AP_CONNECTED_BIT, pdFALSE, pdFALSE, (100 / portTICK_PERIOD_MS));
			//ESP_LOGI("test", "WIFI_MODE_AP0.");
			if(((bits & APP_event_WIFI_AP_CONNECTED_BIT) == APP_event_WIFI_AP_CONNECTED_BIT)) 
			{

				//xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT | APP_event_WIFI_AP_CONNECTED_BIT);
				//sse_data[sse_Least] |= BIT4;
				enWifiMode = WIFI_MODE_AP;
				ESP_LOGI("test", "WIFI_MODE_AP5.");
				//WIFI_Mode_Save(enWifiMode);
				ESP_LOGI("test", "WIFI_MODE_AP4.");
				//stop_OTA_webserver(OTA_server);
				stop_file_server();
				webserver_status = 0x55;
				ESP_LOGI("test", "WIFI_MODE_AP1.");
				//esp_wifi_disconnect();
				ESP_LOGI("test", "WIFI_MODE_AP2.");
				//If connection is lost clear the provisioning available flag and stop wifi and re-start in AccessPoint Mode
				//ESP_LOGI(TAG, "Connection lost! Stopping Wifi to switch to AccessPoint Mode.");
				ESP_ERROR_CHECK(esp_wifi_stop());
				ESP_LOGI("test", "WIFI_MODE_AP3.");
				ESP_LOGI("test", "WIFI_MODE_AP6.");
				vTaskDelay(200 / portTICK_PERIOD_MS);
				ESP_LOGI("test", "WIFI_MODE_AP7.");
				ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
				ESP_LOGI("test", "WIFI_MODE_AP8.");
				WIFI_vConfigSoftAP(&OTA_server);
				ESP_LOGI("test", "WIFI_MODE_AP9.");
				ESP_ERROR_CHECK(esp_wifi_start());
				start_dhcp_server();
			}
			else 
			{
				
				apcount++;
				if((apcount >= 1800) && ((bits & APP_event_WIFI_STA_CONNECTED_BIT) != APP_event_WIFI_STA_CONNECTED_BIT)){
					apcount = 0;
					esp_wifi_connect();
				}
			}
			ESP_LOGI("MyWiFi","enWifiMode == WIFI_MODE_STA\r\n");
		}
		else if(enWifiMode == WIFI_MODE_AP)
		{
			
			enWifiMode = WIFI_Mode_Check();

			//vTaskDelay(60000 / portTICK_PERIOD_MS);
			//enWifiMode = WIFI_MODE_STA;
			//xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT | APP_event_WIFI_AP_CONNECTED_BIT);

			ESP_LOGI("MyWiFi","WIFI_Mode_Check: %d;%d",enWifiMode,stacount);
			
			//received new Provisioning Data, but not tested yet.
			if((enWifiMode == WIFI_MODE_STA) || (stacount >= 720))
			{
				stacount = 0;
				enWifiMode = WIFI_MODE_STA;
				//sse_data[sse_Least] &= ~BIT4;
				//stop_OTA_webserver(OTA_server);
				stop_file_server();
				webserver_status = 0x55;
				// stop DHCP server
				ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
				ESP_LOGI("MyWiFi","New Provisioning data received. Stopping Wifi to switch to Station Mode.");
				ESP_ERROR_CHECK(esp_wifi_stop());
				vTaskDelay(200 / portTICK_PERIOD_MS);
				ESP_LOGI("MyWiFi","Wifi Station initializing!");
				ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
				WIFI_vConfigStation(&OTA_server);
				ESP_ERROR_CHECK(esp_wifi_start());
				bits =xEventGroupWaitBits(APP_event_group, APP_event_WIFI_STA_CONNECTED_BIT, pdFALSE, pdFALSE, (1000 / portTICK_PERIOD_MS)/*min*/ * 240);
				if((bits & APP_event_WIFI_STA_CONNECTED_BIT) != APP_event_WIFI_STA_CONNECTED_BIT){
					xEventGroupSetBits(APP_event_group,APP_event_WIFI_AP_CONNECTED_BIT);
					xEventGroupClearBits(APP_event_group,APP_event_WIFI_STA_CONNECTED_BIT);
				}
			}
			else
			{
				ESP_LOGI("MyWiFi","enWifiMode == WIFI_MODE_AP\r\n");
				stacount++;
			}	
		}
		else{
			
			ESP_LOGI("MyWiFi","enWifiMode == WIFI_MODE_AP\r\n");
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
/*
		if(TimeStatus == 170){
			sse_data[0] |= 0x10000000;
		}
		else{
			sse_data[0] &= 0xEFFFFFFF;
		}
		sse_data[4] = sleep_Time;
		sse_data[5] = AutolWateringTime;
*/		


	}
    vTaskDelete(NULL);
}
#endif