/*
 *  Set DCOCLK to 12 MHz
 *  Set ACLK to REFOCLK at 32.678 kHz
 *  Set MCLK and SMCLK to DCO
 *
 *  Verify clock speed using __delay_cycles macro and blinking LED
 *
 *  Paul Hummel
 */

#include "msp.h"
#include <stdint.h>

#define CYCLES 3000000

uint16_t main(void) {

    WDT_A->CTL = WDT_A_CTL_PW |             // Stop WDT
    WDT_A_CTL_HOLD;

    P1->SEL0 &= ~BIT0;                      // P1.0 set GPIO
    P1->SEL1 &= ~BIT0;
    P1->DIR |= BIT0;                        // P1.0 set as output


    CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    // Select ACLK = REFO, SMCLK = MCLK = DCO
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    CS->KEY = 0;                            // Lock CS module from unintended accesses


    while (1)                               // continuous loop
    {
        P1->OUT ^= BIT0;                    // XOR P1.0
        __delay_cycles(CYCLES);             // 3_000_000 cycles ~ 1s at 3 MHz
    }
}
