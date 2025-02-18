#include <msp430.h>
#include <stdbool.h>

//3.2 - Red 
//3.3 - Green
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

#pragma vector=TIMER_B1_VECTOR
__interrupt void Timer_B0_ISR(void) {
    status_led_count++;

    if (status_led_count > 255) {
        status_led_count = 0;
    }

    // RED PWM
    if (status_led_count < red_count) {
        P3OUT |= BIT2;
    } else {
        P3OUT &= ~BIT2;
    }

    // GREEN PWM
    if (status_led_count < green_count) {
        P3OUT |= BIT3;
    } else {
        P3OUT &= ~BIT3;
    }

    // BLUE PWM
    if (status_led_count < blue_count) {
        P3OUT |= BIT7;
    } else {
        P3OUT &= ~BIT7;
    }

}
