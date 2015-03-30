#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "timer.h"
#include "pin.h"
#include "oc.h"
#include "ui.h"
#include "control_tools.h"

int pos = -10;

void moveServo(_TIMER *self) {
    pos = get_z();
    if (pos == 0) {
        led_on(&led2);
        led_off(&led1);
        led_off(&led3);
        pin_write(&D[6],32767);
    } 
    else if (pos == 10) {
        led_off(&led2);
        led_off(&led1);
        led_on(&led3);
        pin_write(&D[6],65534);
    }
    else if (pos == -10) {
        led_off(&led2);
        led_on(&led1);
        led_off(&led3);
        pin_write(&D[6],0);
    }
    // if (pos == 0) {
    //     led_off(&led2);
    //     led_on(&led1);
    //     pos = 1;
    //     pin_write(&D[6],0);
    // }
    // else if (pos == 1) {
    //     pos = 2;
    // } else if (pos == 2) {
    //     pos = 3;
    // } else if (pos == 3) {
    //     led_off(&led1);
    //     led_on(&led2);
    //     pin_write(&D[6],65534);
    //     pos = 4;
    // } else if (pos == 4) {
    //     pos = 5;
    // } else {
    //     pos = 0;
    // }
}

int16_t main(void) {
    uint16_t btn2ReadState;
    uint16_t btn2CurrState = 0;
    uint16_t counter2;
    uint16_t angle = 0;
    uint16_t decreasing = 0;

    init_clock();
    init_timer();
    init_pin();
    init_oc();
    init_ui();
    init_pot_tracking();

    timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);

    pin_digitalOut(&D[6]);

    timer_every(&timer1,.01,moveServo);

    oc_servo(&oc1, &D[6], &timer2, 20E-3, 1E-3, 2E-3, 0);

    while (1) {
        // btn2ReadState = !sw_read(&sw2);
        // if (btn2CurrState != btn2ReadState) {
        //     counter2++;
        // }
        // else {
        //     counter2 = 0;
        // }

        // if (counter2 > 10) {
        //     counter2 = 0;

        //     btn2CurrState = btn2ReadState;
        //     if (btn2CurrState)
        //     {
        //         led_on(&led1);

        //         if (angle == 65534) {
        //             decreasing = 1;
        //         } 
        //         else if (angle == 0) {
        //             decreasing = 0;
        //         }

        //         if (decreasing) {
        //             angle = angle - 4681;
        //         }
        //         else {
        //             angle = angle + 4681;
        //         }
                
        //         pin_write(&D[6],angle); 
        //     }
        //     else {
        //         led_off(&led1);
        //     }
        // }
    }
}

