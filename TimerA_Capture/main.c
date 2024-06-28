//***************************************************************************************
//  MSP432P401 Demo - TimerA0->CCI2A Capture
//
//  Capture the time between pulses on TA0.2 (P2.5). After every 2 captures,
//  the period is calculated and saved in inputPeriod.
//  MCLK = SMCLK = default DCODIV = 3MHz.
//
//                MSP432P401
//             -----------------
//         /|\|                 |
//          | |             P2.5|<-- TA0.CCI2A
//          --|RST              |
//            |                 |
//            |                 |
//            |                 |
//
//
//  Paul Hummel
//***************************************************************************************
#include "msp.h"
#include <stdint.h>

volatile uint16_t captureValues[2] = {0};
volatile uint16_t captureFlag = 0;

int main(void)
{
	uint16_t capturePeriod;

	WDT_A->CTL = WDT_A_CTL_PW |             // Stop watchdog timer
	WDT_A_CTL_HOLD;

	P2->SEL0 |= BIT5;     // TA0.CCI2A input capture pin, second function
	P2->DIR &= ~BIT5;

	// TimerA0_A2 Setup
	TIMER_A0->CCTL[2] = (TIMER_A_CCTLN_CM_1   | // Capture rising edge,
	                     TIMER_A_CCTLN_CCIS_0 | // Use CCI2A (P2.5),
	                     TIMER_A_CCTLN_CCIE   | // Enable capture interrupt
	                     TIMER_A_CCTLN_CAP    | // Enable capture mode,
	                     TIMER_A_CCTLN_SCS);    // Synchronous capture

	TIMER_A0->CTL |= (TIMER_A_CTL_TASSEL_2       | // SMCLK as clock source,
	                  TIMER_A_CTL_MC__CONTINUOUS | // Continuous mode
	                  TIMER_A_CTL_CLR);            // clear TA0R

	NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);      // enable TimerA ISR

    // Enable global interrupt
	__enable_irq();

	while (1)
	{
        if (captureFlag)
		{
			__disable_irq();    // protect capture times
			// time difference
			capturePeriod = captureValue[1] - captureValue[0];
			captureFlag = 0;    // clear capture flag
			__enable_irq();     // start capturing again
			// Do any time or freq calculations here
		}
	}

}

// TimerA0_N interrupt service routine
void TA0_N_IRQHandler(void)
{
	volatile static uint32_t captureCount = 0;

	if (TIMER_A0->CCTL[2] & TIMER_A_CCTLN_CCIFG)
	{
		captureValue[captureCount] = TIMER_A0->CCR[2];
		captureCount++;

		if (captureCount == 2)
		{
			captureCount = 0;
			captureFlag = 1;
		}

		// Clear the interrupt flag
		TIMER_A0->CCTL[2] &= ~(TIMER_A_CCTLN_CCIFG);
	}
}
