// Program used for classroom demonstration of hardware debugger
// Paul Hummel

#include "msp.h"

int main(void)
{
    // Hold the watchdog
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    // Configuring P1.0 as output for red LED
    // and P1.1 (left button) as input with pull-up resistor.

    P1->SEL0 &= ~(BIT0 + BIT1); // set P1.0 and P1.1 as GPIO
    P1->SEL1 &= ~(BIT0 + BIT1);

    P1->DIR |= BIT0;        // set P1.0 output
    P1->OUT &= ~BIT0;

    P1->DIR &= ~BIT1;       // set P1.1 as input
    P1->REN |= BIT1;        // enable pull up resistor
    P1->OUT |= BIT1;

    P1->IES |= BIT1;        // Interrupt on high-to-low transition
    P1->IFG &= ~BIT1;       // Clear all P1 interrupt flags
    P1->IE  |= BIT1;        // Enable interrupt for P1.1

    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_CCIE; // enable interrupts on CCR[1]
    TIMER_A0->CCR[1] = 30000;  // count for 30000 clock cycles

    TIMER_A0->CTL =  TIMER_A_CTL_TASSEL_1   // select ACLK
                    | TIMER_A_CTL_ID_1      // divide by 2
                    | TIMER_A_CTL_MC_2;     // continuous mode

    NVIC->ISER[0] = (1 << (TA0_N_IRQn & 31));  // enable ISR in NVIC
    NVIC->ISER[1] = (1 << (PORT1_IRQn & 31));  // Enable Port 1 interrupt on the NVIC

    // Enable global interrupt
    __enable_irq();

    while (1);  // loop that does nothing
}

// Port1 ISR
void PORT1_IRQHandler(void)
{
    static uint32_t button_count = 0;

    // Toggling the output on the LED
    if(P1->IFG & BIT1){
        P1->OUT ^= BIT0;
        button_count++;
    }
    P1->IFG &= ~BIT1;
}

// TimerA0 ISR
void TA0_N_IRQHandler(void){

    static uint32_t interrupt_counter = 0;

    if (TIMER_A0->CCTL[1] & TIMER_A_CCTLN_CCIFG){   // check if CCR[1] is cause of interrupt
         TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG; // clear the flag
         TIMER_A0->CCR[1] += 30000;     // interrupt again in 30000 counts

         if (P1->OUT & BIT0) {  // only increment if LED is on
             interrupt_counter++;
         }
    }
}
