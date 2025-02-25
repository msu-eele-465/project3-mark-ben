#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>
#include "../src/statusled.c"
#include "msp430fr2355.h"
#include "../src/ledbar.c"



extern void Timer_B1_ISR(void);

char code[] = "5381";
char keypad_input[4] = {};
volatile int input_index = 0;

const unsigned rowPins[4] = {BIT2, BIT3, BIT5, BIT6};
const unsigned colPins[4] = {BIT0, BIT1, BIT2, BIT3};



const char keypad[4][4] = {                                 // Matrix rep. of keypad for pressedKey function
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

void setup_Heartbeat() {
    // --    LED   --
    
    P6DIR |= BIT6;                                          // P6.6 as OUTPUT
    P6OUT |= BIT6;                                          // Start LED off

    // -- Timer B0 --
    TB0R = 0;
    TB0CCTL0 = CCIE;                                        // Enable Interrupt
    TB0CCR0 = 32820;                                        // 1 sec timer
    TB0EX0 = TBIDEX__8;                                     // D8
    TB0CTL = TBSSEL__SMCLK | MC__UP | ID__4;                // Small clock, Up counter,  D4
    TB0CCTL0 &= ~CCIFG;
}

void rgb_timer_setup() {
    P3DIR |= (BIT2 | BIT7);                                 // Set as OUTPUTS
    P2DIR |= BIT4;
    P3OUT |= (BIT2 | BIT7);                                 // Start HIGH
    P2OUT |= BIT4;

    TB1R = 0;
    TB1CTL |= (TBSSEL__SMCLK | MC__UP);                     // Small clock, Up counter
    TB1CCR0 = 512;                                          // 1 sec timer
    TB1CCTL0 |= CCIE;                                       // Enable Interrupt
    TB1CCTL0 &= ~CCIFG;
}


char pressedKey() {
    int row, col;
    for (row = 0; row < 4; row++) {
        P1OUT &= ~(BIT2 | BIT3 | BIT5 | BIT6);              // Set rows low
        P1OUT |= rowPins[row];                              // current row high

        for(col = 0; col < 4; col++) {                      // Check each column for high
            __delay_cycles(1000);                       
            if((P6IN & colPins[col]) != 0) {                // If column high
                __delay_cycles(1000);                       // Debounce delay
                if((P6IN & colPins[col]) != 0) {            // Check again
                char keyP = keypad[row][col];
                
                while((P6IN & colPins[col]) != 0);          // Wait until key not pressed
                
                return keyP;                                // Update key
                }
            }
        }
    }
    return '\0';                                            // No key entered
}

void check_key() {
    int i, flag = 0;
    if (input_index == 3) {                                 // Only check after 4 digits entered

        for(i=0; i<3; i++) {
            if(keypad_input[i] != code[i]) {
                flag = 1;
            }
        }
        if(flag == 0){                                      // Code is correct
            state_variable = 1;
            input_index = 0;
            memset(keypad_input, 0, sizeof(keypad_input));  // Clear input
        } else {
            state_variable = 0;
            input_index = 0;
            memset(keypad_input, 0, sizeof(keypad_input));  // Clear input
        }
            
            
    }
    
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    
    P1DIR |= (BIT0 | BIT2 | BIT3 | BIT5 | BIT6);        // rows = OUTPUT
    P6DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);              // cols = INPUT
    P6REN |= (BIT0 | BIT1 | BIT2 | BIT3);               // Pulldown resistors on cols
    P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3); 
    P1OUT &= ~(BIT2 | BIT3 | BIT5 | BIT6);              // rows low
    P1OUT &= ~BIT0;
    

    setup_Heartbeat();
    rgb_timer_setup();
    setup_ledbar_timer();
    // Disable the GPIO power-on default high-impedance mdoe to activate
    // previously configure port settings
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        char key = pressedKey();
        if (state_variable == 0 || state_variable == 2) {                      // Locked
            
            if (key != '\0') {                                                 // Check for key

                state_variable = 2;                                            // if key, unlocking
                if (input_index < 3) {                                         
                    keypad_input[input_index] = key;
                    input_index++;
                } else if (input_index == 3) {                                 // if 4 keys, check unlock
                    
                    check_key();
                }
            }   
        } else if (state_variable == 1) {                           // Unlocked

            switch (key) {                                          // Lock if D, otherwise update pattern/base transition period
                case 'D':                                                   
                    state_variable = 0;
                    input_index = 0;
                    change_led_pattern(-1);                         
                    memset(keypad_input, 0, sizeof(keypad_input));  // Clear input
                    break;
                case '0':
                    change_led_pattern(0);
                    break;
                case '1':
                    change_led_pattern(1);
                    break;
                case '2':
                    change_led_pattern(2);
                    break;
                case '3':
                    change_led_pattern(3);
                    break;
                case 'A':
                    if (base_tp > 0.25) {
                        base_tp -= 0.25;
                    }
                    break;
                case 'B':
                    base_tp += 0.25;
                    break;
                default:
                    input_index = 0;
                    memset(keypad_input, 0, sizeof(keypad_input));  // Clear input
                    break;
            }

            input_index = 0;
            memset(keypad_input, 0, sizeof(keypad_input));  // Clear input
        }
        updateled();
    }
}

// ----- ISR ------------------------------

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {
    TB0CCTL0 &= ~CCIFG;
    P6OUT ^= BIT6;
}
