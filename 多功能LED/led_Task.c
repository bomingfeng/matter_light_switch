#include "led_Task.h"
#include "mdf_common.h"

#define LEDC_LS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_GPIO       CONFIG_LED_Lights_GPIO
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

static const char *TAG                       = "LED";

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
        .channel    = LEDC_LS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_LS_CH0_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER,
#if CONFIG_IDF_TARGET_ESP32
        .flags.output_invert = 0
#elif CONFIG_IDF_TARGET_ESP32C3

#endif // CONFIG_IDF_TARGET_*   
};



void ledc_init(void)
{
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

    // Initialize fade service.
    ledc_fade_func_install(0);
}

void led_gpio_init(void)
{
    gpio_reset_pin(CONFIG_LED_Lights_GPIO);
    gpio_set_direction(CONFIG_LED_Lights_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(CONFIG_LED_Lights_GPIO,GPIO_PULLDOWN_ONLY);
    gpio_set_level(CONFIG_LED_Lights_GPIO,0);
}

void LED_Task_init(void)
{
    gpio_reset_pin(CONFIG_Lights_GPIO);
    gpio_set_direction(CONFIG_Lights_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(CONFIG_Lights_GPIO,GPIO_PULLDOWN_ONLY);
    gpio_set_level(CONFIG_Lights_GPIO,0);

    gpio_reset_pin(CONFIG_detectIR_GPIO);
    gpio_set_level(CONFIG_detectIR_GPIO,0);
    gpio_set_direction(CONFIG_detectIR_GPIO,GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_detectIR_GPIO,GPIO_FLOATING);

    //led_gpio_init();
    ledc_init();
}

void led_instructions(void *pvParam)
{
    uint8_t led_state,evenONE;
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
            MDF_LOGI("%s OFF led",(xEventGroupGetBits(APP_event_group) & 0x00000007) == 1 ? "LED_light_Breathe":"Errot");
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
            MDF_LOGI("%s OFF led",(xEventGroupGetBits(APP_event_group) & 0x00000007) == 2 ? "LED_light_100ms":"Errot");       
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
            MDF_LOGI("LED_light_Always_off");  
            break;
        }
        if(gpio_get_level(CONFIG_detectIR_GPIO) != 0x0){
            MDF_LOGI("led ON");
            gpio_set_level(CONFIG_Lights_GPIO,1);
        }
        else{
            gpio_set_level(CONFIG_Lights_GPIO,0);
            MDF_LOGI("OFF led");
        }
        vTaskDelay(time_date / portTICK_PERIOD_MS); 
    }
}
