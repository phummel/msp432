/*
 *  SPI Example using eUSCI_B0 for SPI
 *  P1.5  UCB0CLK   SCLK
 *  P1.6  UCB0SIMO  MOSI
 *  P1.7  UCB0SOMI  MISO
 *
 *  write numbers 1-10 to SPI using polling the IFG flag to wait until the
 *  TXBUF is clear before writing the next number
 *  Interrupts are enabled on receiving from SPI. The ISR will read
 *  the RXBUF and set the value to multicolor LED (P2.0-2)
 *
 *  Paul Hummel
 */

#include "msp.h"
#include <stdint.h>

int main(void)
{
    uint8_t data;
    uint32_t i;

    WDT_A->CTL = WDT_A_CTL_PW |         // Stop watchdog timer
            WDT_A_CTL_HOLD;

    P1->SEL0 |= BIT5 | BIT6 | BIT7;     // Set P1.5, P1.6, and P1.7 as
                                        // SPI pins functionality

    P2->DIR |= BIT0 | BIT1 | BIT2;      // set as output for LED

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI state machine in reset

    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST  | // keep eUSCI in reset
                      EUSCI_B_CTLW0_MST    | // Set as SPI master
                      EUSCI_B_CTLW0_SYNC   | // Set as synchronous mode
                      EUSCI_B_CTLW0_CKPL   | // Set clock polarity high
                      EUSCI_B_CTLW0_UCSSEL_2 | // SMCLK
                      EUSCI_B_CTLW0_MSB;     // MSB first

    EUSCI_B0->BRW = 0x01;               // no div - fBitClock = fBRCLK/(UCBRx)

    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;  // Initialize USCI state machine
    EUSCI_B0->IE |= EUSCI_B_IE_RXIE;          // Enable RX interrupt

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCI_B0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);

    while(1)
    {
        // write numbers 0-7 to SPI. Use TXIFG to verify TXBUF is empty
        for(data=0; data<8; data++)
        {
          // wait for TXBUF to be empty before writing to SPI
          while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
          EUSCI_B0->TXBUF = data;

          for(i=0; i<20000; i++);  // delay loop

        }
    }
}

// SPI interrupt service routine
// Read from SPI and set P2 multicolor LED
void EUSCIB0_IRQHandler(void)
{
    volatile uint8_t RXData;

    if (EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG)  // verify RX interrupt
    {
        RXData = EUSCI_B0->RXBUF;

        P2->OUT &= ~(BIT0 | BIT1 | BIT2); // reset to 0
        P2->OUT |= RXData;                // set data to LEDs
    }
}
