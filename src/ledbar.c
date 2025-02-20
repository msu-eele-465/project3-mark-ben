#include <msp430.h>
#include <string.h>

volatile int pattern = 0;
volatile float base_tp = 1.0;    // Default 1.0s


void setup_ledbar_timer() {
    TB2CCTL0 = CCIE;                    // Enable Timer B2 interrupt
    TB2CCR0 = 32768 * base_tp;  // Set interval 
    TB2CTL = TBSSEL__SMCLK | MC__UP;     // SMCLK, Up mode
}

void update_led_bar() {
    switch(pattern) {
        
    }
}

#pragma vector=TIMER2_B0_VECTOR
__interrupt void Timer_B2_ISR(void) {
    update_led_bar();
    TB2CCR0 = 32768 * base_transition_period;
}