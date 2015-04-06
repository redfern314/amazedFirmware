#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include "oc.h"
#include <stdio.h>

// Tests out the control_tools library

int balls_seen = 0;

void accept_ball() {
    printf("Saw a new winning ball!\n");
    balls_seen++;
}

void setup() {
	init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    init_oc();
    led_on(&led1); led_on(&led2); led_on(&led3);

    init_ball_tracking(&accept_ball);

    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);
}

int16_t main(void) {
    setup();

    while (1) {
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            printf("Number of balls seen so far: %d\n", balls_seen);
        }
    }
}