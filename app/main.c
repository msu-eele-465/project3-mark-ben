#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>

#define code "5381"

volatile int state_variable = 0;                        // 0 = Locked, 1 = Unlocked, 2 = Unlocking
char keypad_input[5] = {};
volatile int input_index = 0;

const unsigned rowPins[4] = {BIT2, BIT3, BIT5, BIT6};
const unsigned colPins[4] = {BIT0, BIT1, BIT2, BIT3};

volatile int status_led_count = 0;
volatile int red_count = 0;
volatile int green_count = 0;
volatile int blue_count = 0;

const char keypad[4][4] = {                             // Matrix rep. of keypad for pressedKey function
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

void setup_Heartbeat() {
    // --    LED   --
    
    P6DIR |= BIT6;                                      // P6.6 as OUTPUT
    P6OUT &= ~BIT6;                                     // Start LED off

    // -- Timer B0 --

    TB0CCTL0 = CCIE;                                    // Enable Interrupt
    TB0CCR0 = 32820;                                    // 1 sec timer
    TB0EX0 = TBIDEX__8;                                 // D8
    TB0CTL = TBSSEL0__SMCLK | MC__UP | ID__4;           // Small clock, Up counter,  D4
}

char pressedKey(void) {
    int row, col;
    for (row = 0; row < 4; row++) {
        P1OUT &= ~(BIT2 | BIT3 | BIT5 | BIT6);          // Set rows low
        P1OUT |= rowPins[row];                          // current row high
        __delay_cycles(1000);                           // C delay cycle function

        for(col = 0; col < 4; col++) {
            if(P1IN & colPins[col]) {
                return keypad[row][col];
                }
            }
        }
    }
    return '\0';                                         // No key entered
}

void check_key() {
    if (input_index == 4) {                             // Only check after 4 digits entered
        keypad_input[4] = '\0';
        if (strcmp(keypad_input, code) == 0) {
            state_variable = 1;
        } else {
            input_index = 0;
        }
    }
}

void rgb_timer_setup() {
    P3DIR |= (BIT2 | BIT3 | BIT7);                      // Set as OUTPUTS
    P3OUT |= (BIT2 | BIT3 | BIT7);                      // Start HIGH

    TB1CCTL0 = CCIE;                                    // Enable Interrupt
    TB1CCR0 = 8205;                                     // 1 sec timer
    TB1CTL = TBSSEL0__SMCLK | MC__UP;                   // Small clock, Up counter
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    
    P1DIR |= (BIT2 | BIT3 | BIT5 | BIT6);               // rows = OUTPUT
    P6DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);              // cols = INPUT
    P6REN |= (BIT0 | BIT1 | BIT2 | BIT3);               // Pulldown resistors on cols
    P1OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);              // rows low

    setup_Heartbeat();
    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        if (state_variable == 0) {                      // Locked
            char key = pressedKey();
            if (key != '\0') {
                state_variable = 2;
                if (input_index < 4) {
                    keypad_input[input_index++] = key;
                }
                check_key();
            }   
        } else if (state_variable == 1) {               // Unlocked
            if (key == 'D') {
                state_variable = 0;
                input_index = 0;
            }
        }
    }
}

// ----- ISR ------------------------------

// ----- Timer B0 Heartbeat ---------------

#pragma vector=TIMER_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {
    P6OUT ^= BIT6;
}

// ---- END TimerB0 -----------------------