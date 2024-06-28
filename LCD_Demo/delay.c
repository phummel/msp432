// Delay functions based on running clock speed of 3 MHz
// Paul Hummel

#include "delay.h"

void delay_ms(uint32_t time)
{
    uint32_t i, j;
    for (i=time; i>0; i--)
        for (j=200; j>0; j--);
}

void delay_us(uint32_t time)
{
    uint32_t i;
    for (i=time; i>0; i--);
}

