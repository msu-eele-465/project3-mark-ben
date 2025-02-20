#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>
#include "../src/statusled.c"
#include "msp430fr2355.h"
#include <string.h>



extern void Timer_B1_ISR(void);

char code[] = "5381";
char keypad_input[4] = {};
volatile int input_index = 0;


const unsigned rowPins[4] = {BIT2, BIT3, BIT5, BIT6};
const unsigned colPins[4] = {BIT0, BIT1, BIT2, BIT3};



const char keypad[4][4] = {                             // Matrix rep. of keypad for pressedKey function
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

void setup_Heartbeat() {
    // --    LED   --
    
    P6DIR |= BIT6;                                      // P6.6 as OUTPUT
    P6OUT |= BIT6;                                     // Start LED off

    // -- Timer B0 --

    TB0CCTL0 = CCIE;                                    // Enable Interrupt
    TB0CCR0 = 32820;                                    // 1 sec timer
    TB0EX0 = TBIDEX__8;                                 // D8
    TB0CTL = TBSSEL__SMCLK | MC__UP | ID__4;           // Small clock, Up counter,  D4
}



char pressedKey() {
    int row, col;
    for (row = 0; row < 4; row++) {
        P1OUT &= ~(BIT2 | BIT3 | BIT5 | BIT6);          // Set rows low
        P1OUT |= rowPins[row];                          // current row high

        for(col = 0; col < 4; col++) {
            __delay_cycles(10000);
            if(!(P6IN & colPins[col])) {
                __delay_cycles(1000);
                if(!(P6IN & colPins[col])) {
                char keyP = keypad[row][col];
                
                while(!(P6IN & colPins[col]));
                
                return keyP;
                }
            }
        }
    }
    return '\0';                                         // No key entered
}

void check_key() {
    
    if (input_index == 3) {                             // Only check after 4 digits entered

        if (strncmp(keypad_input, code, 4) == 0) {
            
            state_variable = 1;
        } else {
            input_index = 0;
            memset(keypad_input, 0, sizeof(keypad_input));  // Clear the input buffer
        }
    }
}

void rgb_timer_setup() {
    P3DIR |= (BIT2 | BIT3 | BIT7);                      // Set as OUTPUTS
    P3OUT |= (BIT2 | BIT3 | BIT7);                      // Start HIGH

    TB1CCTL1 = CCIE;                                    // Enable Interrupt
    TB1CCR1 = 8205;                                     // 1 sec timer
    TB1CTL = TBSSEL__SMCLK | MC__UP;                     // Small clock, Up counter
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    
    P1DIR |= (BIT0 | BIT2 | BIT3 | BIT5 | BIT6);        // rows = OUTPUT
    P6DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);              // cols = INPUT
    P6REN |= (BIT0 | BIT1 | BIT2 | BIT3);               // Pulldown resistors on cols
    P1OUT &= ~(BIT2 | BIT3 | BIT5 | BIT6);              // rows low
    P1OUT &= ~BIT0;

    setup_Heartbeat();
    rgb_timer_setup();
    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        char key = pressedKey();
        if (state_variable == 0 || state_variable == 2) {                      // Locked
            
            if (key != '\0') {
                
                state_variable = 2;
                if (input_index < 3) {
                    keypad_input[input_index++] = key;
                } else if (input_index == 3) {
                    
                    check_key();
                }
            }   
        } else if (state_variable == 1) {               // Unlocked
            P1OUT ^= BIT0;
            if (key == 'D') {
                state_variable = 0;
                input_index = 0;
            }
        }

        updateled();
    }
}

// ----- ISR ------------------------------

// ----- Timer B0 Heartbeat ---------------

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {
    TB0CCTL0 &= ~CCIFG;
    P6OUT ^= BIT6;
}

// ---- END TimerB0 -----------------------