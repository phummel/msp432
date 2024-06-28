//***************************************************************************************
//  C Code Demo - Finite State Machine (Framework)
//
//  Simple Vending Machine Example -
//    4 states - Idle / Count_Money / Vending / Change
//    3 events - Input_Money / Make_Selection / Coin_Return
//
//  This is not a complete program. It is meant to be used as
//  an example framework to show a method for creating an FSM
//  in C using enum types and switch statements. The FSM events
//  are driven by interrupts.
//
//  Paul Hummel
//***************************************************************************************
#include <stdint.h>

enum states {
  IDLE,
  COUNT_MONEY,
  VENDING,
  CHANGE
} state_type;

enum events {
  INPUT_MONEY,
  MAKE_SELECTION,
  COIN_RETURN
} event_type;


int main(void)
{
    state_type state = IDLE;
    event_type event;
    
    uint32_t money;

    while (1)
    {
      switch (state) {
        case IDLE:                      // initial state
          if (event = INPUT_MONEY) {    // if money input start counting
            state = COUNT_MONEY;
          } else if (event = MAKE_SELECTION) {  // if a selection is made
            state = VENDING;
          }
          break;
          
        case COUNT_MONEY:
          money += input_money;           // keep a total of input money
          if (event = MAKE_SELECTION) {   // selection is made
            state = VENDING;
          } else if (event = COIN_RETURN){  // money return selected
            state = CHANGE;
          }
          state = IDLE;   // after counting money, return to idle
          break;
        
        case VENDING:
          price = price_lookup(input_selection);  // lookup price
          
          if (price < money){
            dispense(input_selection);  // dispense selection
            money -= price;             // calculate change
            state = CHANGE;             // go to dispense change
          
            
          } else if (price = money){    // exact change given
            dispense(input_selection);  // dispense selection
            money = 0;                  // no change remaining
            state = IDLE;
          
            
          } else {              // not enough money
            display(price);     // display cost
            state = IDLE;
          }
          break;
        
        case CHANGE:
          return_change(money);   // dispense change or money return
          money = 0;              // reset money total
          state = IDLE;           // go back to idle
          break;
        
        default:  // failsafe
          state = IDLE;
          break;
      }
      
    }

}

// Interrupt on money interface
void MoneyIRQ(void) {
  event = INPUT_MONEY;
  input_money = read_input();
}

// Interrupt on selection panel
void SelectionIRQ(void) {
  event = MAKE_SELECTION;
  input_selection = read_selection();
}

// Interrupt for coin return
void CoinReturnIRQ(void) {
  event = COIN_RETURN;
}
