/*
 *  Function library for 4-bit parallel LCD
 *
 *  Data is connected to the upper 4 bits of port 2
 *  EN / RS / RW are connected to the upper 3 bits of port 3
 *
 *  LCD_init()  - Initialize the LCD display
 *  LCD_nib_cmd(unsigned char command)  - issue command to LCD
 *  LCD_write(unsigned char letter)
 *
 * Paul Hummel
 */

#include "msp.h"

#define CPU_FREQ 3000000
#define __delay_us(t_us) (__delay_cycles((((uint64_t)t_us)*CPU_FREQ) / 1000000))

#define RS BIT5     // P3.5
#define RW BIT6     // P3.6
#define EN BIT7     // P3.7
#define CONTROL_MASK  (RS | RW | EN)

#define DB7 BIT7    // Data Nibble connected to
#define DB6 BIT6    // P2.7 - 2.4
#define DB5 BIT5
#define DB4 BIT4
#define DATA_MASK     (DB7 | DB6 | DB5 | DB4)

// Define common LCD command functions
#define CLR_DISP      0x01    // Clear display
#define HOME          0x02    // Send cursor to home position
#define WAKE          0x30    // initial wake command
#define MODE_8_BIT    0x30    // LCD Mode options
#define MODE_4_BIT    0x20
#define MODE_2_LINE   0x28
#define CURSOR_ON     0x0A
#define CURSOR_BLINK  0x09
#define DISPLAY_ON    0x0C
#define CURSOR_RIGHT  0x06    // Cursor moves to right
#define CURSOR_LEFT   0x04    // Cursor moves to left
#define SET_CURSOR    0x80    // Set cursor position to SET_CURSOR | ADDRESS
#define LINE_TWO      0x40    // Address of 2nd line
#define LINE_THREE    0x14
#define LINE_FOUR     0x54

void LCD_init(void);
void LCD_nib_cmd(uint8_t command);
void LCD_char_write(uint8_t letter);
void LCD_write(const char* message);

int main(void) {

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    LCD_init();
    LCD_char_write('H');
    LCD_char_write('e');
    LCD_char_write('l');
    LCD_char_write('l');
    LCD_char_write('o');

    LCD_nib_cmd(SET_CURSOR | LINE_TWO); // Move cursor to 2nd line
    LCD_write("World!");

    // off screen for 2-line displays
    LCD_nib_cmd(SET_CURSOR | LINE_THREE); // Move cursor to 3rd line
    LCD_write("Microcontrollers");
    LCD_nib_cmd(SET_CURSOR | LINE_FOUR); // Move cursor to 4th line
    LCD_write("Rock!");

    while (1);  // infinite loop
}

// Function to initialize the LCD display
// 4-bit mode, 2 line, cursor on
void LCD_init(void) {

    // Setup GPIO for P2 and P3 to use LCD
    P3->SEL0 &= ~(CONTROL_MASK);    // Control signals RS, RW, EN
    P3->SEL1 &= ~(CONTROL_MASK);    // Setup as GPIO outputs
    P3->DIR |= (CONTROL_MASK);

    P2->SEL0 &= ~(DATA_MASK);       // Data bits DB7-DB4
    P2->SEL1 &= ~(DATA_MASK);       // Setup as GPIO outputs
    P2->DIR |= (DATA_MASK);

    P3->OUT &- ~EN;                 // Set Enable low
    __delay_us(40000);              // wait >40 ms for LCD to power up

    // single nibble wake up initialization command
    P3->OUT &= ~(RS | RW);              // RS, RW = 0
    P2->OUT &= ~(DATA_MASK);            // Set Data
    P2->OUT |= (WAKE & DATA_MASK);

    P3->OUT |= EN;          // Pulse E for > 460 ns
    __delay_us(1);
    P3->OUT &= ~EN;
    __delay_us(40);         // delay > 37 us

    LCD_nib_cmd(MODE_4_BIT | MODE_2_LINE);      // set 4-bit data, 2-line
    LCD_nib_cmd(MODE_4_BIT | MODE_2_LINE);      // set 4-bit data, 2-line
    LCD_nib_cmd(DISPLAY_ON | CURSOR_ON | CURSOR_BLINK);    // display and cursor on
    LCD_nib_cmd(CLR_DISP);                      // clear screen
    LCD_nib_cmd(CURSOR_RIGHT);                  // move cursor right after each char
}

// Function to send a single command to the LCD in 8-bit mode
void LCD_nib_cmd(uint8_t command) {

    uint8_t cmdLow;

    // shift lower 4 bits of command to upper
    cmdLow = ((command & 0x0F) << 4);

    P3->OUT &= ~(EN | RS | RW);        // reset control signals

    // upper 4 bits of command
    P2->OUT &= ~(DATA_MASK);           // reset upper 4 bits
    P2->OUT |= (command & DATA_MASK);  // set upper 4 bits of command

    P3->OUT |= EN;                     // Pulse E for > 460 ns
    __delay_us(1);
    P3->OUT &= ~EN;
    __delay_us(1);

    P2->OUT &= ~(DATA_MASK);           // reset upper 4 bits
    P2->OUT |= (cmdLow & DATA_MASK);   // set lower 4 bits of command

    P3->OUT |= EN;                     // Pulse E for > 460 ns
    __delay_us(1);
    P3->OUT &= ~EN;

    if (command < 4)
        __delay_us(1700);     // command 1 and 2 need up to 1.52ms
    else
        __delay_us(40);       // all others 37 us
}

// Function to write a single character at the cursor location
void LCD_char_write(uint8_t letter) {

    P3->OUT |= RS;                     // RS = 1
    P3->OUT &= ~(EN | RW);             // EN, R/W = 0

    P2->OUT &= ~(DATA_MASK);           // reset upper 4 bits
    P2->OUT |= (letter & DATA_MASK);   // set upper 4 bits of letter

    P3->OUT |= EN;                     // pulse E > 460 ns
    __delay_us(1);
    P3->OUT &= ~EN;
    __delay_us(1);

    // shift lower 4 bits of letter to upper
    letter = (letter << 4);

    P2->OUT &= ~(DATA_MASK);           // reset upper 4 bits
    P2->OUT |= (letter & DATA_MASK);   // set lower 4 bits of letter

    P3->OUT |= EN;                     // pulse E > 460 ns
    __delay_us(1);
    P3->OUT &= ~EN;

    __delay_us(40);                    // wait for LCD to display
}

// Function to write a full string of characters to the LCD
void LCD_write(const char* message){

    for (uint16_t letter = 0; message[letter] != '\0'; letter++)
        LCD_char_write(message[letter]);
}
