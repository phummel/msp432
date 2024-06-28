/*
 *  Function library for 8-bit parallel LCD
 *  LCD_init()  - Initialize the LCD display
 *  LCD_cmd(unsigned char command)  - issue command to LCD
 *  LCD_write(unsigned char letter)
 *
 *  Paul Hummel
 */

#include "msp.h"
#include "delay.h"  // From A2 - define delay functions

#define RS BIT5     /* P3.5 mask */
#define RW BIT6     /* P3.6 mask */
#define EN BIT7     /* P3.7 mask */

// Define common LCD command functions
#define CLR_DISP      0x01    // Clear display
#define HOME          0x02    // Send cursor to home position
#define MODE_8_BIT    0x30    // LCD Mode options
#define MODE_2_LINE   0x28
#define CURSOR_ON     0x0A
#define CURSOR_BLINK  0x09
#define DISPLAY_ON    0x0C
#define CURSOR_RIGHT  0x06    // Cursor moves to right
#define CURSOR_LEFT   0x04    // Cursor moves to left
#define SET_CURSOR    0x80    // Set cursor position to SET_CURSOR | ADDRESS

void LCD_init(void);
void LCD_command(unsigned char command);
void LCD_write(unsigned char letter);

int main(void) {
    LCD_init();
    LCD_write('H');
    LCD_write('e');
    LCD_write('l');
    LCD_write('l');
    LCD_write('o');

    LCD_command(SET_CURSOR | 0x40); // Move cursor to 2nd line
    LCD_write('W');
    LCD_write('o');
    LCD_write('r');
    LCD_write('l');
    LCD_write('d');
    LCD_write('!');

    while (1);  // infinite loop
}


// Function to initialize the LCD display
// 8-bit mode, 2 line, cursor on
void LCD_init(void) {

    // Setup GPIO for P3 and P4 to use LCD
    P3->SEL0 &= ~(RS | RW | EN);
    P3->SEL1 &= ~(RS | RW | EN);
    P4->SEL0 = 0x00;
    P4->SEL1 = 0x00;
    P3->DIR |= RS | RW | EN; // make P3 pins output for control
    P4->DIR = 0xFF;          // make P4 pins output for data

    P3->OUT &- ~EN;           // Set Enable low
    delay_ms(50);             // wait >40 ms for LCD to power up

    LCD_command(MODE_8_BIT | MODE_2_LINE);  // wake up initialization
    LCD_command(MODE_8_BIT | MODE_2_LINE);  // set 8-bit data, 2-line
    LCD_command(DISPLAY_ON | CURSOR_ON);    // display and cursor on
    LCD_command(CLR_DISP);                  // clear screen
    LCD_command(CURSOR_RIGHT);              // move cursor right after each char
}

// Function to send a single command to the LCD in 8-bit mode
void LCD_command(unsigned char command) {
    P3->OUT &= ~(RS | RW);  // RS = 0, RW = 0
    P4->OUT = command;      // put command on data bus

    P3->OUT |= EN;          // Pulse E for > 460 ns
    delay_us(0);
    P3->OUT &= ~EN;

    if (command < 4)
        delay_ms(2);         // command 1 and 2 need up to 1.52ms
    else
        delay_us(40);       // all others 37 us
}

// Function to write a single character at the cursor location
void LCD_write(unsigned char letter) {
    P3->OUT |= RS;        // RS = 1
    P3->OUT &= ~RW;       // R/W = 0
    P4->OUT = letter;     // put letter on bus
    P3->OUT |= EN;        // pulse E > 460 ns
    delay_us(0);
    P3->OUT &= ~EN;
    delay_us(2);           // wait for LCD to display
}
