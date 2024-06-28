//******************************************************************************
//  MSP432P401 Demo - ADC14, Sample A1, AVcc Ref
//
//   Description: ADC14 is running in single conversion mode, sampling from A1
//   with reference to AVcc using the sample and hold timer. On initialization
//   the SC bit is set to start a conversion. On the interrupt when completing
//   a conversion the value is saved in a global and a flag is set. In main,
//   the while loop uses the global flag to reset the SC bit to start and new
//   conversion and save 20 samples before processing. After 20 samples, the
//   max, min, and average of the samples is calculated and printed to the
//   console using printf because I am being lazy and am a horrible, terrible
//   programmer. DO NOT DO THIS!!! After processing the 20 samples, repeats.
//
//
//                MSP432P401x
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//        >---|P5.4/A1          |
//
//   Paul Hummel
//   Cal Poly
//   Spring 2020
//******************************************************************************
#include "msp.h"
#include <stdio.h>

#define ADC_SAMPLE_SIZE 20

static volatile uint8_t ADC_Flag = 0;
static volatile uint16_t ADC_Sample = 0;


void main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // halt watchdog timer

    uint16_t adc_buffer[ADC_SAMPLE_SIZE];
    uint8_t adc_buffer_index = 0;
    uint16_t adc_avg, adc_min, adc_max;
    uint32_t adc_total;

    P5->SEL1 |= BIT4;                   // Configure P5.4 for ADC
    P5->SEL0 |= BIT4;

    // Sampling time, S&H=192, ADC14 on
    ADC14->CTL0 = ADC14_CTL0_SHT0__4
                | ADC14_CTL0_SHP
                | ADC14_CTL0_SSEL__SMCLK
                | ADC14_CTL0_ON;
    ADC14->CTL1 = ADC14_CTL1_RES_3;         // Use sampling timer, 12-bit conversion results

    ADC14->MCTL[0] = ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
    ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt
    ADC14->CTL0 |= ADC14_CTL0_ENC;          // Enable conversion

    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);   // Enable ADC interrupt in NVIC module

    __enable_irq();     // Enable global interrupt

    // Start sampling/conversion
    ADC14->CTL0 |= ADC14_CTL0_SC;

    while (1)
    {
        if (ADC_Flag) { // conversion done

            ADC_Flag = 0;                               // save sample, clear flag, move index
            adc_buffer[adc_buffer_index] = ADC_Sample;
            adc_buffer_index++;

            if (adc_buffer_index == ADC_SAMPLE_SIZE) {   // have all the samples wanted, so begin processing

                adc_total = 0;  // clear measurements
                adc_max = 0;
                adc_min = 0xFFFF;

                for (adc_buffer_index = 0; adc_buffer_index < ADC_SAMPLE_SIZE; adc_buffer_index++){
                    adc_total += adc_buffer[adc_buffer_index];

                    if (adc_buffer[adc_buffer_index] > adc_max) // check for max
                        adc_max = adc_buffer[adc_buffer_index];

                    if (adc_buffer[adc_buffer_index] < adc_min) // check for min
                        adc_min = adc_buffer[adc_buffer_index];
                }

                adc_avg = adc_total / ADC_SAMPLE_SIZE;

                printf("Average is %d\n",adc_avg);  // horrible and should not be done!!!!
                printf("Minimum is %d\n",adc_min);
                printf("Maximum is %d\n",adc_max);
                printf("Delta   is %d\n\n",adc_max-adc_min);
                adc_buffer_index = 0;                   // reset buffer to fill again

                //__delay_cycles(100000);

            }

            ADC14->CTL0 |= ADC14_CTL0_SC;   // start next sample
        }
    }
}

// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {

    if (ADC14->IFGR0 & ADC14_IFGR0_IFG0) {  // MEM[0] triggered interrupt
        ADC_Sample = ADC14->MEM[0] ;        // save result and set global flag
        ADC_Flag = 1;
    }
}
