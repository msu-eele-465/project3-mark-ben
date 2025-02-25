#include <msp430.h>
#include <string.h>

volatile int pattern = 0;
volatile int step[4] = {0, 0, 0, 0};
volatile float base_tp = 1.0;    // Default 1.0s


const uint patern_1[2] = {0b10101010, 0b01010101};
const uint patter_3[6] = {0b00011000, 0b00100100,
                          0b01000010, 0b10000001,
                          0b01000010, 0b00100100};

void setup_ledbar_timer() {
    TB2CCTL0 = CCIE;                    // Enable Timer B2 interrupt
    TB2CCR0 =  (int) ((1.0 / 32768) * base_tp);  // Set interval 
    TB2CTL = TBSSEL__SMCLK | MC__UP;     // SMCLK, Up mode
}

void update_led_bar() {
    uint led_pins = 0;
    
    switch(pattern) {
    case 0:
        led_pins = 0b10101010;
        break;
    case 1:
        led_pins = pattern_1[step[pattern]];
        step[pattern] = (step[pattern] + 1) % 2;
        break;
    case 2:
        led_pins = step[pattern];
        step[pattern] = (step[pattern] + 1) % 255;
        break;
    case 3:
        led_pins = patern_3[step[pattern]];
        step[pattern] = (step[pattern] + 1) % 6;
        break; 
    }

    change_led_pattern(led_pins);
}

void change_led_pattern(int new_pattern) {
    if (new_pattern == pattern) {
        step[pattern] = 0;
    }

    pattern = new_pattern;
}

void update_led_bar_pins(uint pins) {
    if ((current_pins & 0b00000001) == 1) {
        P2OUT |= BIT0;
    } else {
        P2OUT &= ~BIT0;
    }

    if ((current_pins & (0b00000001 << 1)) == 1) {
        P2OUT |= BIT2;
    } else {
        P2OUT &= ~BIT2;
    }

    if ((current_pins & (0b00000001 << 2)) == 1) {
        P4OUT |= BIT0;
    } else {
        P4OUT &= ~BIT0;
    }

    if ((current_pins & (0b00000001 << 3)) == 1) {
        P4OUT |= BIT6;
    } else {
        P4OUT &= ~BIT6;
    }

    if ((current_pins & (0b00000001 << 4)) == 1) {
        P4OUT |= BIT7;
    } else {
        P4OUT &= ~BIT7;
    }

    if ((current_pins & (0b00000001 << 5)) == 1) {
        P4OUT |= BIT4;
    } else {
        P4OUT &= ~BIT4;
    }

    if ((current_pins & (0b00000001 << 6)) == 1) {
        P2OUT |= BIT5;
    } else {
        P2OUT &= ~BIT5;
    }

    if ((current_pins & (0b00000001 << 7)) == 1) {
        P2OUT |= BIT0;
    } else {
        P2OUT &= ~BIT0;
    }
}

#pragma vector=TIMER2_B0_VECTOR
__interrupt void Timer_B2_ISR(void) {
    update_led_bar();
    TB2CCR0 = (int) ((1.0 / 32768) * base_tp);
}