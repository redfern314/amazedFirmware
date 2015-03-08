
#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "ui.h"
#include "timer.h"
#include "pin.h"
#include "oc.h"
#include "uart.h"
#include <stdio.h>

#define true                    1
#define false                   0

void readJoystick() {
    uint16_t data = pin_read(&A[5]); // get the ADC reading
    printf("%f\r\n",data);
}

int16_t main(void) {
    init_clock();
    init_ui();
    init_timer();
    init_pin();
    init_oc();
    init_uart();

    led_on(&led3);

    timer_every(&timer1,0.01,&readJoystick);

    while (1) {
        // hang out and wait for interrupts
    }
}