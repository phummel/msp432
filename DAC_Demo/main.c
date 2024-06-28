/*
 *  Example using SPI to connect to MCP 4921
 *  P4.4  Port1.4   CS
 *  P1.5  UCB0CLK   SCK
 *  P1.6  UCB0SIMO  SDI
 *
 *  LDAC - ground to always set low, no buffering
 *  MISO / SIMO is not needed because no data coming from DAC
 *
 *  Write values 0 to 4095 to
 *
 * Paul Hummel
 */

#include "msp.h"
#include <stdint.h>

#define GAIN BIT5
#define SHDN BIT4
#define DAC_CS  BIT4

int main(void)
{
    uint16_t data;
    uint8_t hiByte, loByte;
    uint32_t i;

    WDT_A->CTL = WDT_A_CTL_PW |         // Stop watchdog timer
            WDT_A_CTL_HOLD;

    P1->SEL0 |= BIT5 | BIT6 | BIT7;     // Set P1.5, P1.6, and P1.7 as
                                        // SPI pins functionality

    P4->SEL0 &= ~DAC_CS;
    P4->SEL1 &= ~DAC_CS;

    P4->DIR |= DAC_CS;                      // set as output for CS
    P4->OUT |= DAC_CS;

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI state machine in reset

    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST    |  // keep eUSCI in reset
                      EUSCI_B_CTLW0_MST      |  // Set as SPI master
                      EUSCI_B_CTLW0_SYNC     |  // Set as synchronous mode
                      EUSCI_B_CTLW0_CKPL     |  // Set clock polarity high
                      EUSCI_B_CTLW0_UCSSEL_2 |  // SMCLK
                      EUSCI_B_CTLW0_MSB;        // MSB first

    EUSCI_B0->BRW = 0x01;              // div by 2 fBitClock = fBRCLK / UCBRx

    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;  // Initialize USCI state machine

    while(1)
    {
        // write numbers 0-7 to SPI. Use TXIFG to verify TXBUF is empty
        for(data=0; data<4096; data++)
        {
          // set the low and high bytes of data
          loByte = 0xFF & data;         // mask just low 8 bits
          hiByte = 0x0F & (data >> 8);  // shift and mask bits for D11-D8
          hiByte |= (GAIN | SHDN);      // set the gain / shutdown control bits

          P4->OUT &= ~DAC_CS; // Set CS low

          // wait for TXBUF to be empty before writing high byte
          while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
          EUSCI_B0->TXBUF = hiByte;

          // wait for TXBUF to be empty before writing low byte
          while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
          EUSCI_B0->TXBUF = loByte;

          // wait for RXBUF to be empty before changing CS
          while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));

          P4->OUT |= DAC_CS; // Set CS high

          // delay before changing output voltage
          for (i=10; i>0; i--);

        }
    }
}
