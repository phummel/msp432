// Program that uses the comparator to trigger an interrupt that
// is detectable by cycling through all colors of the RGB LED
// Paul Hummel

#include "msp.h"

#define COLOR_LED (BIT0 | BIT1 | BIT2)

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	COMP_E1->CTL0 = COMP_E_CTL0_IPEN        // enable + input comparator
	              | COMP_E_CTL0_IPSEL_0;    // select C1.0 P6.7

	COMP_E1->CTL1 = COMP_E_CTL1_ON     // turn comp on
	              | COMP_E_CTL1_F      // filter output
	              | COMP_E_CTL1_FDLY_3;

	COMP_E1->CTL2 = (17 << COMP_E_CTL2_REF1_OFS)    // set REF1 = 14/32 * 3.3 = 1.44
	              | COMP_E_CTL2_RS_1                // select Vcc for resistor ladder input
	              | COMP_E_CTL2_RSEL                // set Reference to go to (-) input
	              | (18 << COMP_E_CTL2_REF0_OFS);   // set REF0 = 18/32 * 3.3 = 1.86

	COMP_E1->CTL3 = COMP_E_CTL3_PD0;                // disable input for C1.0

	COMP_E1->INT = COMP_E_INT_IE;           // enable interrupt for rising edge

	P6->SEL0 |= BIT7;   // select C1.0 for P6.7
	P6->SEL1 |= BIT7;

	P7->SEL0 |= BIT2;   // select C1 Out on P7.2
	P7->SEL1 &= ~BIT2;
	P7->DIR |= BIT2;

	P2->DIR |= COLOR_LED;       // use RGB LED as indicator
	P2->OUT &= ~(COLOR_LED);

	NVIC->ISER[0] = (1 << (COMP_E1_IRQn & 31));
	__enable_irq();

	while(1); // do nothing
}

// comparator ISR changes the color of the RGB LED on every trigger
void COMP_E1_IRQHandler(void){
    volatile static uint8_t color = 0;

    if (COMP_E1->INT & COMP_E_INT_IFG) {
        P2->OUT &= ~COLOR_LED;
        P2->OUT |= (color & COLOR_LED);
        color++;
        COMP_E1->INT &= ~COMP_E_INT_IFG;
    }
}
