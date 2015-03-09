#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include <stdio.h>

uint8_t string[40];

int16_t main(void) {
    init_clock();
    init_pin();
    init_timer();
    init_uart();  // Default is 19200 baud
    init_ui();
    setup_motor_shield();

    printf("Hello World!\n");
    led_on(&led1); led_on(&led2); led_on(&led3);

    printf("What is your name? ");
    uart_gets(&uart1, string, 40);
    printf("Hello %s!\n", string);

    printf("Press sw1 and sw2 to drive the motors\n");

    while (1) {
        if (!sw_read(&sw1)) {
            pin_clear(&D[5]);
            pin_set(&D[6]);
        }
        else if (!sw_read(&sw2)) {
            pin_set(&D[5]);
            pin_clear(&D[6]);
        }
        else {
            pin_clear(&D[5]);
            pin_clear(&D[6]);
        }
    }
}
