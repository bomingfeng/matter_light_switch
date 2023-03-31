#include "detectIR_control.h"

extern uint32_t sse_data[sse_len];

int get_detectIR_status(void)
{
    int status = gpio_get_level(CONFIG_detectIR_GPIO);
    if(status != 0){
        sse_data[0] |= 0x20000000;
    }
    else{
        sse_data[0] &= 0xDFFFFFFF;
    }
    return status;
}