#include <p24FJ128GB206.h>
#include <stdio.h>
#include "config.h"
#include "common.h"
#include "oc.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"

uint8_t string[40];

int16_t main(void) {
    init_clock();
    init_oc();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    setup_motor_shield();
    pin_set(&D[6]);

    timer_setPeriod(&timer1, 1);
    timer_start(&timer1);

    int duty_cycle = 0;
    // void oc_pwm(_OC *self, _PIN *out, _TIMER *timer,
    //             float freq (Hz), uint16_t duty)
    oc_pwm(&oc1, &D[3], NULL, 10000, duty_cycle << 6);  // D[3] is tri-stating
    // oc_pwm(&oc1, &D[6], NULL, 10000, duty_cycle << 6);  // D[6] is on/off

    printf("Hello World!\n");
    led_on(&led1); led_on(&led2); led_on(&led3);

    printf("What is your name? ");
    uart_gets(&uart1, string, 40);
    printf("Hello %s!\n", string);

    printf("Testing PWM.\n");

    while (1) {
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            duty_cycle += 128;
            if (duty_cycle > 1023) duty_cycle = 0;
            printf("Duty cycle: %d\n", duty_cycle);
            pin_write(&D[3], duty_cycle << 6);  // 1023 is stopped, 0 is full
            // pin_write(&D[6], duty_cycle << 6);
        }
    }
}
