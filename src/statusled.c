#include <msp430.h>
#include <stdbool.h>


volatile int state_variable = 0;                        // 0 = Locked, 1 = Unlocked, 2 = Unlocking

volatile int status_led_count = 0;
volatile int red_count = 0;
volatile int green_count = 0;
volatile int blue_count = 0;

//3.2 - Red 
//2.4 - Green
//3.7 - Blue

// This function will update the current status of the led
void updateled(void) {
    switch (state_variable) {
        case 0:
            red_count = 0xc4;
            green_count = 0x3e;
            blue_count = 0x1d;
            break;
        case 1:
            red_count = 0x1d;
            green_count = 0xa2;
            blue_count = 0xc4;
            break;
        case 2:
            red_count = 0xc4;
            green_count = 0x92;
            blue_count = 0x1d;
            break;
    }
}

#pragma vector=TIMER1_B0_VECTOR
__interrupt void Timer_B1_ISR(void) {

    TB1CCTL0 &= ~CCIFG;

    status_led_count++;

    if (status_led_count > 0xff) {
        status_led_count = 0;
    }

    // RED PWM
    if (status_led_count <= red_count) {
        P3OUT |= BIT2;
    } else {
        P3OUT &= ~BIT2;
    }

    // GREEN PWM
    if (status_led_count <= green_count) {
        P2OUT |= BIT4;
    } else {
        P2OUT &= ~BIT4;
    }

    // BLUE PWM
    if (status_led_count <= blue_count) {
        P3OUT |= BIT7;
    } else {
        P3OUT &= ~BIT7;
    }
}
