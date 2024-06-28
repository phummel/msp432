// Program designed to measure the timing for various C operations with
// different variable types. By toggling a pin before and after function
// call (P2.0) and toggling a second pin before and after the operation,
// multiple timing aspects can be measured.
// P2.0 (1)->Call function->P1.0 (1)->operation->P1.0 (0)->return->P1.0 (0)
//
// Paul Hummel

#include "msp.h"
#include <math.h>

#define var_type uint8_t

var_type TestFunction(var_type num);

void main(void) {

    var_type mainVar;

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;   // Stop watchdog timer

    CS->KEY = CS_KEY_VAL;
    CS->CTL0 = CS_CTL0_DCORSEL_3;    // set DCO to 12 MHz
    CS->CTL1 = (CS_CTL1_DIVM__1 | CS_CTL1_SELS__DCOCLK | CS_CTL1_SELM__DCOCLK);    // MCLK select DCO
    CS->KEY = 0;

    P1->SEL1 &= ~BIT0;          //set P1.0 as simple I/O
    P1->SEL0 &= ~BIT0;
    P1->DIR |= BIT0;            //set P1.0 as output

    P2->SEL1 &= ~BIT0;          //set P2.0 as simple I/O
    P2->SEL0 &= ~BIT0;
    P2->DIR |= BIT0;            //set P2.0 as output pins

    P4->SEL0 |= BIT3;           // set MCLK out for measurement of clock speed
    P4->SEL1 &= ~BIT3;
    P4->DIR |= BIT3;

    P2->OUT |= BIT0;            // turn on Blue LED
    mainVar = TestFunction(15); // test function for timing
    P2->OUT &= ~BIT0;           // turn off Blue LED

    while(1)       // infinite loop to do nothing
        mainVar++; // increment mainVar to eliminate not used warning
}

// Function used to measure the execution time for function call / return
// that varies by parameter variable type.
var_type TestFunction(var_type num) {

    var_type testVar;

    P1->OUT |= BIT0;    // turn RED LED on

    /* Replace this line with the operation to be measured */
    testVar = (num);
    /*******************************************************/

    P1->OUT &= ~BIT0;   // turn RED LED off
    return testVar;
}
