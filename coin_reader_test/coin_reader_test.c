#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include <stdio.h>

/*
Counts coins as they come through and prints that count to serial
Assumes the coin counter is plugged into Analog 0, at the moment
*/ 

#define COIN_VOLTAGE_LEVEL  40  // The analog voltage level that indicates
                                // a coin, from 0-1023
#define COIN_READ_PIN       5   // Analog pin

volatile int read_voltage;  // Analog voltage read on the coin counter
volatile int coin_in_slot;  // A coin is currently in the machine
volatile int new_coin = 0;  // True when there is a new new, unreported coin
volatile int total_coin_count = 0;  // Counts each coin as it passes through

void readCoinCounter(_TIMER *self) {
    read_voltage = pin_read(&A[COIN_READ_PIN]) >> 6;
    if (read_voltage > COIN_VOLTAGE_LEVEL) {
        coin_in_slot = 1;
    } else if (read_voltage < COIN_VOLTAGE_LEVEL) {
        if (coin_in_slot == 1) {
            new_coin += 1;
            total_coin_count += 1;
        }
        coin_in_slot = 0;
    }
}

void setup() {
    init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    led_on(&led1); led_on(&led2); led_on(&led3);

    timer_setPeriod(&timer1, 0.2);
    timer_start(&timer1);
    pin_analogIn(&A[COIN_READ_PIN]);
    timer_every(&timer2, 0.05, readCoinCounter);
}

int16_t main(void) {
    setup();

    while (1) {
        if (new_coin > 0) {
            printf("Saw a new coin!\n");
            new_coin -= 1;
        }

        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            printf("Number of coins seen so far: %d\n", total_coin_count);
        }
    }
}