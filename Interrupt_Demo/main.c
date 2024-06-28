// Program to introduce interrupts using the MSP432 Launcpad dev board
// Left button on the Launchpad will cause the red LED to toggle on and off
// P1.1 -> Button (active low)
// P1.0 -> Red LED
//
// Paul Hummel

#include "msp.h"

void main(void)
{
    // Hold the watchdog
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    P1->SEL0 &= ~(BIT0 | BIT1);   // set button and red LED to GPIO
    P1->SEL1 &= ~(BIT0 | BIT1);

    P1->DIR &= ~(BIT1);           // set button as input
    P1->REN |= (BIT1);            // enable resistor on button input
    P1->OUT |= (BIT1);            // set resistor to pull up

    P1->DIR |= (BIT0);            // set red LED as output
    P1->OUT &= ~(BIT0);           // initialize red LED off

    P1->IE |= BIT1;               // enable interrupts on button
    P1->IES |= BIT1;              // set interrupt on falling edge
    P1->IFG &= ~(BIT1);           // clear the flag if inadvertently set

    NVIC->ISER[1] = (1 << 3);     // enable P1 interrupts in NVIC

    __enable_irq();               // enable interrupts globally

    while(1);
}

// GPIO Port 1 ISR
// Configured to trigger on the falling edge of P1.1 and toggle Red LED (P1.0)
void PORT1_IRQHandler(void){    // P1 interrupt handler

    if (P1->IFG & BIT1)     // check if interrupt from button
    {
        P1->OUT ^= BIT0;    // toggle red LED
        P1->IFG &= ~BIT1;   // clear interrupt flag
    }
}








