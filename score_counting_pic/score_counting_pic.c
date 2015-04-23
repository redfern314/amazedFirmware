#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include "oc.h"
#include "spi.h"
#include <stdio.h>

// Tests out the control_tools library

int win_balls_seen = 0;
int lose_balls_seen = 0;

void accept_ball(int which_breakbeam) {
    printf("Saw a new ball! Breakbeam: %d\n", which_breakbeam);
    if (which_breakbeam == 0) {
        win_balls_seen++;
    } else {
        lose_balls_seen++;
    }
}

void start_game(void) {
    pin_set(&D[SCORE_START_STOP_PIN]);
}

void end_game(int win) {
    pin_clear(&D[SCORE_START_STOP_PIN]);
    // if (win) {
    //     //do stuff related to win
    // } else if {
    //     //do stuff related to not win
    // }
}

void setup() {
	init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    init_oc();
    init_spi();
    led_on(&led1); led_on(&led2); led_on(&led3);

    init_coin_tracking(&start_game);
    init_seven_segment();


    timer_every(&timer1,.1,display_elapsed_time);

    init_ball_tracking(&end_game);

    // timer_setPeriod(&timer1, 0.5);
    // timer_start(&timer1);
}

int16_t main(void) {
    setup();

    while (1) {
        // if (timer_flag(&timer1)) {
        //     timer_lower(&timer1);
        //     printf("Number of balls seen so far:\n\tWinning: %d\n\tLosing: %d\n",
        //            win_balls_seen, lose_balls_seen);
        // }
    }
}