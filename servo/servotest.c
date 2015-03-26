#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "timer.h"
#include "pin.h"
#include "oc.h"
#include "ui.h"

uint16_t pos = 0;

void moveServo(_TIMER *self) {
    if (pos == 0) {
        led_off(&led2);
        led_on(&led1);
        pos = 1;
        pin_write(&D[6],0);
    }
    else if (pos == 1) {
        pos = 2;
    } else if (pos == 2) {
        pos = 3;
    } else if (pos == 3) {
        led_off(&led1);
        led_on(&led2);
        pin_write(&D[6],65534);
        pos = 4;
    } else if (pos == 4) {
        pos = 5;
    } else {
        pos = 0;
    }
}

int16_t main(void) {
    init_clock();
    init_timer();
    init_pin();
    init_oc();
    init_ui();

    pin_digitalOut(&D[6]);

    led_on(&led1);

    timer_every(&timer3,1,moveServo);

    oc_servo(&oc1, &D[6], &timer2, 20E-3, 1E-3, 2E-3, 0);
    while (1) {
        
    }
}

