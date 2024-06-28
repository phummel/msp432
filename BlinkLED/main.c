// Example program written in class to introduce MSP432
// Paul Hummel

#include "msp.h"

int main(void) {
    int i;

    // stop watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    P1->SEL0 &= BIT0;                   // P1.0 set as GPIO
    P1->SEL1 &= BIT0;
    P1->DIR |= BIT0;                    // P1.0 set as output

    while (1)                           // continuous loop
    {
        P1->OUT ^= BIT0;                // Blink P1.0 LED
        for (i = 20000; i > 0; i--);    // Delay
    }
}
