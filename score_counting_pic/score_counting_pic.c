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

uint16_t best_score = 400;
int won = 0;
int lost = 0;

void start_game(void) {
    won = 0;
    lost = 0;
    pin_set(&D[SCORE_START_STOP_PIN]);
    led_on(&led3);
}

void end_game(int win) {
    pin_clear(&D[SCORE_START_STOP_PIN]);
    led_off(&led3);
    timer_stop(&timer1);
    timer_setPeriod(&timer2, 0.6);
    timer_start(&timer2);
    if (win) {
        won = 1;
    } else {
        lost = 1;
    }
}

void start_seven_segment(void) {
    write_data(0x09,0xff); 
    timer_every(&timer1,.12,display_elapsed_time);
}

void setup() {
	init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    init_oc();
    init_spi();

    led_on(&led2);

    init_coin_tracking(&start_game);
    init_ball_tracking(&end_game);
    init_vacuum_tracking(&start_seven_segment);

    init_seven_segment();
}

int16_t main(void) {
    setup();

    int count = 1;

    while (1) {

        if (won) {
            if (timer_flag(&timer2)) {
                timer_lower(&timer2);
                if (count % 3 == 1) {
                    write_data(0x09,240);  
                    write_data(4,94);       //G
                    write_data(3,126);      //O
                    write_data(2,126);      //O
                    write_data(1,61);       //d                         
                } else if (count % 3 == 2) {
                    write_data(0x09,240);  
                    write_data(4,0);        //blank
                    write_data(3,60);       //J
                    write_data(2,126);      //O
                    write_data(1,31);       //b 
                } else {
                    write_data(0x09,0xff); 
                    uint16_t score = get_time(); 
                    for (int i = 1; i < 5; i++) {
                        uint8_t digit = score % 10;
                        score = score / 10;

                        write_data(i,digit);
                    }
                } 
                count++;
            }  
        } else if (lost) {
            if (timer_flag(&timer2)) {
                timer_lower(&timer2);
                if (count % 2 == 1) {
                    write_data(1,11);       //L
                    write_data(2,5);        //O
                    write_data(3,0);        //S
                    write_data(4,13);       //E
                } else {
                    uint16_t score = get_time(); 
                    for (int i = 1; i < 5; i++) {
                        uint8_t digit = score % 10;
                        score = score / 10;

                        write_data(i,digit);
                    }
                } 
                count++;
            } 
        }
    }
}