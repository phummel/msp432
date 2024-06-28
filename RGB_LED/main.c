// Demo program that cycles through all of the possible colors
// of the RGB LED (P2.0 - P2.2)

#include "msp.h"
#include <stdint.h>

void main(void) {
    uint16_t i, color = 0;

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;            // Stop WDT

    P1->SEL0 &= ~BIT0;                    // P1.0 set as GPIO
    P1->SEL1 &= ~BIT0;
    P1->DIR |= BIT0;                      // P1.0 set as output

    P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);    // P2.0, 2.1, 2.2 set as GPIO
    P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);
    P2->DIR |= (BIT0 | BIT1 | BIT2);      // P2.0, 2.1, 2.2 set as output

    while (1)                             // continuous loop
    {
        P1->OUT ^= BIT0;                  // Blink P1.0 LED
        P2->OUT &= ~(BIT0 | BIT1 | BIT2); // clear P2.0-2.2
        P2->OUT |= (color & 0x07);        // Set with lower 3 bits of color
        for (i = 65535; i > 0; i--);      // Delay
        color++;                          // change color
    }
}
