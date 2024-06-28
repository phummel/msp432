//*******************************************************************************
//  MSP432P401 Demo - Timer1_A3, PWM TA1.1-2, Up Mode, DCO SMCLK
//
//  This program generates two PWM outputs on P6.6, P6.7 using Timer1_A
//  configured for up mode. The value in CCR0, 32764, defines the PWM period
//  and the values in CCR3 and CCR4 the PWM duty cycles. Using 32 kHz ACLK as
//  TACLK, the timer period is ~1s with a 25% duty cycle on P6.6 and 75% on
//  P7.6.
//
//
//           MSP432P401
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |     P6.6/TA2.3|--> CCR3 - 75% PWM
//        |     P6.7/TA2.4|--> CCR4 - 25% PWM
//
//  Paul Hummel
//******************************************************************************
#include "msp.h"

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW |   // Stop WDT
            WDT_A_CTL_HOLD;

    P6->SEL0 |= BIT6 | BIT7;      // Configure P6.6~7 for TimerA2
    P6->SEL1 &= ~(BIT6 | BIT7);
    P6->DIR |= BIT6 | BIT7;       // P6.6~7 output for TA2.3~4

    TIMER_A2->CCR[0] = 32765 - 1;               // PWM Period
    TIMER_A2->CCTL[3] = TIMER_A_CCTLN_OUTMOD_7; // CCR1 reset/set
    TIMER_A2->CCR[3] = 24574;                   // CCR1 PWM duty cycle
    TIMER_A2->CCTL[4] = TIMER_A_CCTLN_OUTMOD_7; // CCR2 reset/set
    TIMER_A2->CCR[4] = 8191;                    // CCR2 PWM duty cycle
    TIMER_A2->CTL = TIMER_A_CTL_SSEL__ACLK |    // ACLK
            TIMER_A_CTL_MC__UP |                // Up mode
            TIMER_A_CTL_CLR;                    // Clear TAR

    // Enter LPM0
    __sleep();
    __no_operation();                           // For debugger
}
