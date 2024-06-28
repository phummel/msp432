/* Keypad.c: Matrix keypad scanning
 *
 * This program scans a 4x3 matrix keypad and returns the number of the
 * key that is pressed. The getkey() function is non-blocking, so it will
 * return 0xFF if no key is pressed to allow 0 to be used for key 0.
 * * key will return 10 and # will return 12.
 *
 * The getkey() function assumes that only a single key is pressed. If keys
 * on multiple columns are pressed, the key on the farthest left column will
 * be detected and the others will be ignored. If multiple keys in the same
 * column are pressed, the function will return an incorrect value.
 *
 * Port 4 is used for the keypad:
 *  Columns are connected to 4.4 - 4.7, Rows 4.0 - 4.3
 *  1 key is at column 1 (4.4) and row 1 (4.0)
 * The columns are cycled while the rows are read, so pull down resistors
 * are enabled for the rows (4.0 - 4.3).
 *
 * Paul Hummel
 */

#include "msp.h"
#include <stdint.h>
#include <stdio.h>

#define COL1  BIT4
#define COL2  BIT5
#define COL3  BIT6
#define ROW1  BIT0
#define ROW2  BIT1
#define ROW3  BIT2
#define ROW4  BIT3

#define COL_MASK (COL1 | COL2 | COL3)
#define ROW_MASK (ROW1 | ROW2 | ROW2 | ROW4)
#define RGB_MASK 0x07


void keypad_init(void);
uint8_t keypad_getkey(void);

int main(void) {
    uint8_t key, rgb;

    // setup GPIO for multicolor LED (2.0-2.2) for key bits 2-0
    P2->DIR |= RGB_MASK;        // make pins output
    P2->OUT &= ~(RGB_MASK);     // turn LED off

    P1->DIR |= BIT0;            // use red led for key bit 3
    P1->OUT &= ~BIT0;           // turn LED off

    keypad_init();              // setup gpio pins for keypad

    while(1) {
        key = keypad_getkey();  // read the keyboard value
        //printf("%d\n",key);   // print key value to console (debug)
        rgb = key & 0x07;       // only keep bottom 3 bits

        // zero bottom 3 bits before being set by key value
        P2->OUT = (P2->OUT &= ~(RGB_MASK)) | rgb;
        key = (key >> 3);                     // shift bit 4 to bit 0
        P1->OUT = (P1->OUT &= ~BIT0) | key;   // only set bit 0 with key
    }
}

/* this function initializes Port 4 that is connected to the keypad.
 * All pins are configured as GPIO input pin. The row pins have
 * the pull-down resistors enabled.
 */
void keypad_init(void) {
    P4->DIR = 0;            // make all pins an input
    P4->REN |= ROW_MASK;    // enable resistor for row pins
    P4->OUT &= ~(ROW_MASK); // make row pins pull-down
}

/*
 * This is a non-blocking function to read the keypad.
 * If a key is pressed, it returns that key value 0-9. * is 10, # is 12
 * If no key is pressed, it returns 0xFF
 * Port 4.0 - 4.3 are used as inputs and connected to the rows. Pull-down
 * resistors are enabled so when no key is pressed, these pins are pulled low
 *
 * The Port 4.4 - 4.6 are used as outputs that drives the keypad columns.
 * First all columns are driven high and the input pins are read. If no key is
 * pressed, they will read zero because of the pull-down resistors. If no key
 * is pressed, return 0xFF. If the value is non-zero, determine which key is
 * being pressed.
 * To determine which key is being pressed, the program proceeds to drive one
 * column high at a time and read the input pins (rows). Knowing which row is
 * high and which column is active, the program can decide which key is pressed
 *
 */
uint8_t keypad_getkey(void) {
    uint8_t row, col, key;

    /* check to see any key pressed */
    P4->DIR |= COL_MASK;  // make the column pins outputs
    P4->OUT |= COL_MASK;  // drive all column pins high
    _delay_cycles(25);                // wait for signals to settle

    row = P4->IN & (ROW_MASK);   // read all row pins

    if (row == 0)           // if all rows are low, no key pressed
        return 0xFF;

    /* If a key is pressed, it gets here to find out which key.
     * It activates one column at a time and reads the input to see
     * which row is active. */

    for (col = 0; col < 3; col++) {
        // zero out bits 6-4
        P4->OUT &= ~(COL_MASK);

        // shift a 1 into the correct column depending on which to turn on
        P4->OUT |= (COL1 << col);
        _delay_cycles(25);            // wait for signals to settle

        row = P4->IN & (ROW_MASK); // mask only the row pins

        if (row != 0) break;      // if the input is non-zero, key detected
    }

    P4->OUT &= ~(COL_MASK);   // drive all columns low
    P4->DIR &= ~(COL_MASK);   // disable the column outputs

    if (col == 3)   return 0xFF;        // if we get here, no key was detected

    // rows are read in binary, so powers of 2 (1,2,4,8)
    if (row == 4) row = 3;
    if (row == 8) row = 4;

    /*******************************************************************
     * IF MULTIPLE KEYS IN A COLUMN ARE PRESSED THIS WILL BE INCORRECT *
     *******************************************************************/

    // calculate the key value based on the row and columns where detected
    if (col == 0) key = row*3 - 2;
    if (col == 1) key = row*3 - 1;
    if (col == 2) key = row*3;

    if (key == 11)  key = 0; // fix for 0 key

    return key;
}
