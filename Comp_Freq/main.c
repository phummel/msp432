// Program that uses the internal comparator to create a square wave
// that feeds into Timer A capture inputs to measure the period of
// the input signal and calculate its frequency
// Paul Hummel

#include "msp.h"
#include <stdio.h>

#define FREQ_1_5        ((uint32_t)0x00000000)          /*!< Nominal DCO Frequency Range (MHz): 1 to 2 */
#define FREQ_3          ((uint32_t)0x00010000)          /*!< Nominal DCO Frequency Range (MHz): 2 to 4 */
#define FREQ_6          ((uint32_t)0x00020000)          /*!< Nominal DCO Frequency Range (MHz): 4 to 8 */
#define FREQ_12         ((uint32_t)0x00030000)          /*!< Nominal DCO Frequency Range (MHz): 8 to 16 */
#define FREQ_24         ((uint32_t)0x00040000)          /*!< Nominal DCO Frequency Range (MHz): 16 to 32 */
#define FREQ_48         ((uint32_t)0x00050000)          /*!< Nominal DCO Frequency Range (MHz): 32 to 64 */

#define CPU_FREQ 24000000
#define __delay_us(t_us) (__delay_cycles((((uint64_t)t_us)*CPU_FREQ) / 1000000))

#define COLOR_LED (BIT0 | BIT1 | BIT2)
#define TIMER_COUNTS CPU_FREQ / 8

void setDCO(uint32_t Frequency);

volatile uint32_t period_count = 0;
volatile uint8_t freq_update = 0;

void main(void)
{
    uint32_t freq = 0;

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    setDCO(FREQ_24);    // speed up the MCU

    COMP_E1->CTL0 = COMP_E_CTL0_IPEN        // enable + input comparator
                  | COMP_E_CTL0_IPSEL_0;    // select C1.0 P6.7

    COMP_E1->CTL1 = COMP_E_CTL1_ON          // turn comp on
                  | COMP_E_CTL1_F           // filter output
                  | COMP_E_CTL1_FDLY_3;

    COMP_E1->CTL2 = (14 << COMP_E_CTL2_REF1_OFS)    // set REF1 = 14/32 * 3.3 = 1.44
                  | COMP_E_CTL2_RS_1                // select Vcc for resistor ladder input
                  | COMP_E_CTL2_RSEL                // set Reference to go to (-) input
                  | (18 << COMP_E_CTL2_REF0_OFS);   // set REF0 = 18/32 * 3.3 = 1.86

    COMP_E1->CTL3 = COMP_E_CTL3_PD0;                // disable input for C1.0

    P6->SEL0 |= (BIT7);   // select C1.0 for P6.7
    P6->SEL1 |= (BIT7);

    P7->SEL0 |= BIT2;    // select C1 Out on P7.2
    P7->SEL1 &= ~BIT2;
    P7->DIR |= BIT2;

    P2->DIR |= COLOR_LED;       // use RGB LED as indicator
    P2->OUT &= ~(COLOR_LED);

    TIMER_A0->CCTL[3] = TIMER_A_CCTLN_CM__RISING // capture rising edge
                      | TIMER_A_CCTLN_CCIS_1     // select CCI3B = C1.OUT
                      | TIMER_A_CCTLN_SCS        // synchronous captures
                      | TIMER_A_CCTLN_CAP        // capture mode
                      | TIMER_A_CCTLN_CCIE;      // enable interrupts

    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK     // select SMCLK
                  | TIMER_A_CTL_ID__8           // divide by 8 (total 64)
                  | TIMER_A_CTL_IE              // enable rollover interrupt
                  | TIMER_A_CTL_MC__CONTINUOUS; // continous mode

    NVIC->ISER[0] = (1 << (TA0_N_IRQn & 31));   // enable TIMER_A0 interrupt CCR[3]
    __enable_irq();

    //__delay_cycles(CPU_FREQ / 2);    // wait 0.5s for comparator to be ready
    __delay_us(500000);                // wait 0.5s for comparator to be ready

    while(1) {
        if (freq_update) {
            freq = TIMER_COUNTS / period_count;
            //printf("f:%d\n",freq);
            if (freq != 9){                 // debug testing real time results
                P2->OUT ^= BIT0;            // with toggling GPIO pin
                //printf("f:%d\n",freq);
            }
            freq_update = 0;
        }
    }
}

// TimerA ISR measures clock counts between rising edges of
// comparator output to measure frequency
void TA0_N_IRQHandler(void){

    volatile static uint16_t overflow = 0;
    volatile static uint16_t past     = 0;
    volatile static uint16_t current  = 0;

    if (TIMER_A0->CTL & TIMER_A_CTL_IFG) {    // roll over
        TIMER_A0->CTL &= ~(TIMER_A_CTL_IFG);
        overflow++;
    }

    if (TIMER_A0->CCTL[3] & TIMER_A_CCTLN_CCIFG) {      // capture
        TIMER_A0->CCTL[3] &= ~(TIMER_A_CCTLN_CCIFG);    // clear flag

        if (TIMER_A0->CCTL[3] & TIMER_A_CCTLN_COV) {     // did we miss cap?
            TIMER_A0->CCTL[3] &= ~(TIMER_A_CCTLN_COV);   // clear flag
            past = TIMER_A0->CCR[3];    // updated past with the most recent capture value
            overflow = 0;               // reset overflow count
            P2->OUT ^= BIT2;            // debug LED to see if missing captures
        }
        else {
            current = TIMER_A0->CCR[3];     // read current capture
            period_count = (0x10000 * overflow) + current - past;  // calculate counts between edges

            past = current;     // update past with the current for the next capture
            overflow = 0;       // reset overflow count
            freq_update = 1;      // freq has been updated
        }
    }
}

// Sets the digitally controlled clock
void setDCO(uint32_t Frequency){

    if (Frequency == FREQ_48){  // change power modes
        /* Transition to VCORE Level 1: AM0_LDO --> AM1_LDO */
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
        PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

        /* Configure Flash wait-state to 1 for both banks 0 & 1 */
        FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL &
                ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) | FLCTL_BANK0_RDCTL_WAIT_1;
        FLCTL->BANK1_RDCTL  = (FLCTL->BANK0_RDCTL &
                ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) | FLCTL_BANK1_RDCTL_WAIT_1;
    }

    CS->KEY = CS_KEY_VAL ;      // Unlock CS module for register access
    CS->CTL0 = 0;               // Reset tuning parameters
    CS->CTL0 = Frequency;       // Set DCO to 48MHz

    CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK) |
            CS_CTL1_SELM_3;
    CS->KEY = 0;              // Lock CS module from unintended accesses

    return;
}
