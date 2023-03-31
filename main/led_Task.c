#include "led_Task.h"

//#define CONFIG_LED_Lights_GPIO 10

static const char *TAG                       = "LED";

#if CONFIG_IDF_TARGET_ESP8266
/*
    * Prepare individual configuration
    * for each channel of LED Controller
    * by selecting:
    * - controller's channel number
    * - output duty cycle, set initially to 0
    * - GPIO number where LED is connected to
    * - speed mode, either high or low
    * - timer servicing selected channel
    *   Note: if different channels use one timer,
    *         then frequency and bit_num of these channels
    *         will be the same
    */
ledc_channel_config_t LED_Lights= {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = CONFIG_LED_Lights_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0,
#if CONFIG_IDF_TARGET_ESP32
        .flags.output_invert = 0
#elif CONFIG_IDF_TARGET_ESP32C3

#endif // CONFIG_IDF_TARGET_*   
};

#elif CONFIG_IDF_TARGET_ESP32C3
static ledc_channel_config_t LED_Lights = {
    .gpio_num = CONFIG_LED_Lights_GPIO,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_1,
    .duty = 0,
    .hpoint = 0,
};
#endif 

/*
#if CONFIG_IDF_TARGET_ESP32
        
#elif CONFIG_IDF_TARGET_ESP32C3

#endif 
*/

void ledc_init(void)
{
#if CONFIG_IDF_TARGET_ESP8266
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&LED_Lights);


#elif CONFIG_IDF_TARGET_ESP32C3
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // timer mode
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .timer_num = LEDC_TIMER_1, // timer index
        .freq_hz = 5000, // frequency of PWM signal
        .clk_cfg = LEDC_AUTO_CLK, // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config(&LED_Lights);

#endif 
    // Initialize fade service.
    ledc_fade_func_install(0);
}

void LED_Task_init(void)
{
    ledc_init();
}

void led_instructions(void *pvParam)
{
    uint32_t duty = 8000,time_date = 100;
      
    while(1) 
    {
        switch (xEventGroupGetBits(APP_event_group) & 0x00000007)
        {
        case 1:
            if(duty != 0)
            {
                duty = 0;
            }
            else{
                duty = 8000;
            }
            ledc_set_fade_with_time(LED_Lights.speed_mode, \
                                    LED_Lights.channel, duty, 3000);
            ledc_fade_start(LED_Lights.speed_mode, \
                            LED_Lights.channel, LEDC_FADE_NO_WAIT);
            time_date = 3500;
            break;
        case 2:    
            if(duty != 0)
            {
                duty = 0;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }
            else{
                duty = 8000;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }   
            time_date = 100;      
            break;
        case 3:    
            if(duty != 0)
            {
                duty = 0;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }
            else{
                duty = 8000;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }   
            time_date = 500;      
            break;
        case 4:    
            if(duty != 0)
            {
                duty = 0;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }
            else{
                duty = 8000;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }   
            time_date = 1000;      
            break;
        case 5:    
            if(duty != 0)
            {
                duty = 0;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }
            else{
                duty = 8000;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }   
            time_date = 1500;      
            break;
        case 6:    
            if(duty != 0)
            {
                duty = 0;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }
            else{
                duty = 8000;
                ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
                ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            }   
            time_date = 2000;      
            break;
        case 7:    
            ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 8000);
            ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            time_date = 2000;      
            break;
        default:
            ledc_set_duty(LED_Lights.speed_mode, LED_Lights.channel, 0);
            ledc_update_duty(LED_Lights.speed_mode, LED_Lights.channel);
            time_date = 2000;
            break;
        }
        vTaskDelay(time_date / portTICK_PERIOD_MS); 
    }
}
