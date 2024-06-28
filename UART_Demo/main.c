//******************************************************************************
//   MSP432P401 Demo - eUSCI_A0 UART echo at 115.2 kHz baud using BRCLK = 3MHz
//
//  Echoes back double characters received via a PC serial port.
//  if 'R', 'G', 'B', or 'W' is entered, the text color is changed accordingly
//  SMCLK/ DCO at 3 MHz is used as a clock source
//
//                MSP432P401
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.3/UCA0TXD|----> PC (echo)
//            |     P1.2/UCA0RXD|<---- PC
//            |                 |
//
//  Paul Hummel
//******************************************************************************
#include "msp.h"

#define RED_TXT   "[31m"
#define GREEN_TXT "[32m"
#define BLUE_TXT  "[34m"
#define WHITE_TXT "[37m"
#define BLINK_TXT "[5m"
#define CLEAR_TXT "[0m"
#define RET_HOME  "[H"
#define ESC_CHAR  0x1B

void UART_write_string(const char* print_string);
void UART_esc_code(const char* esc_code);

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW |             // Stop watchdog timer
    WDT_A_CTL_HOLD;

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
    EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

    // Baud Rate calculation
    // 3000000/(115200) = 26.041667
    // Fractional portion = 0.041667
    // User's Guide Table 21-4: UCBRSx = 0x00
    // UCBRx = int (26.041667 / 16) = 1
    // UCBRFx = int (((26.041667/16)-1)*16) = 10

    EUSCI_A0->BRW = 1;                      // Using baud rate calculator
    EUSCI_A0->MCTLW = (10 << EUSCI_A_MCTLW_BRF_OFS) |
    EUSCI_A_MCTLW_OS16;

    // Configure UART pins
    P1->SEL0 |= (BIT2 | BIT3);                // set 2-UART pin as secondary function

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

    // Enable global interrupt
    __enable_irq();

    UART_esc_code(CLEAR_TXT);       // clear text attributes
    UART_esc_code("[5B");           // move down 5 lines
    UART_esc_code("[5C");           // move right 5 spaces
    UART_esc_code(RED_TXT);         // change font to red
    UART_write_string("Hello ");
    UART_esc_code(BLINK_TXT);       // turn on blink!!!
    UART_esc_code("[46m");          // change background to cyan
    UART_write_string("World!");
    UART_esc_code(RET_HOME);        // return home
    UART_esc_code(CLEAR_TXT);       // clear text attributes
    UART_write_string("Input: ");

    while(1); // do nothing (will echo characters with interrupt)
}

// Function to print a NULL terminated string of characters to UART
void UART_write_string(const char* print_string){

    // print each character until reaching a NULL character
    for (uint16_t letter = 0; print_string[letter] != 0; letter++)
    {
        // wait for TXBUF to be empty
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
        EUSCI_A0->TXBUF = print_string[letter];
    }
}

// Function to print a NULL terminated string compriseing a VT-100 ESC code
void UART_esc_code(const char* esc_code){

    // wait for TXBUF to be empty and send ESC character
    while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
    EUSCI_A0->TXBUF = ESC_CHAR;

    // print each character until reaching a NULL character
    for (uint16_t letter = 0; print_string[letter] != 0; letter++)
    {
        // wait for TXBUF to be empty
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
        EUSCI_A0->TXBUF = print_string[letter];
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void)
{
    uint8_t character;

    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
    {
        character = EUSCI_A0->RXBUF;
        switch (character){
            case 'R':
                UART_esc_code(RED_TXT);    // make text red
                break;
            case 'G':
                UART_esc_code(GREEN_TXT);  // make text green
                break;
            case 'B':
                UART_esc_code(BLUE_TXT);   // make text blue
                break;
            case 'W':
                UART_esc_code(WHITE_TXT);  // make text white
                break;
            default:
                // wait for TXBUF to be empty
                while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
                EUSCI_A0->TXBUF = character;  // echo character
        }
    }
}

