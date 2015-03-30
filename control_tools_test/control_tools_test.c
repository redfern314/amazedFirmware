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

int coins_read = 0;

void accept_coin() {
    printf("Saw a new coin!\n");
    coins_read++;
}

void setup() {
	init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    init_oc();
    led_on(&led1); led_on(&led2); led_on(&led3);

    init_coin_tracking(&accept_coin);

    init_pot_tracking();
	timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);  

    init_z_axis();
    timer_every(&timer4, .001, set_z);

    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);
}

int16_t main(void) {
    setup();

    while (1) {
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            // printf("Number of coins seen so far: %d\n", get_coins());
            printf("Control inputs:\n");
            printf("\tX: %d\n", get_x());
            printf("\tY: %d\n", get_y());
            printf("\tZ: %d\n", get_z());
            // printf("X: %d\tY: %d\n", get_x(), get_y());
        }
    }
}