// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#include <p24FJ128GB206.h>
#include <math.h>
#include <stdio.h>
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "oc.h"
#include "spi.h"


#ifndef SCORE_PIC

// Limit switch initialization
// These pins are numbered from the barrel jack
_PIN pin0, pin1, pin2, pin3;
_PIN *LIMIT_Y_FRONT_PIN, *LIMIT_Y_BACK_PIN, *RELAY_PIN;

void init_extra_pins() {
    pin_init(&pin0, (uint16_t *)&PORTG, (uint16_t *)&TRISG, (uint16_t *)NULL,
             9, -1, 8, 27, (uint16_t *)&RPOR13);
    pin_init(&pin1, (uint16_t *)&PORTG, (uint16_t *)&TRISG, (uint16_t *)NULL,
             8, -1, 8, 19, (uint16_t *)&RPOR9);
    pin_init(&pin2, (uint16_t *)&PORTB, (uint16_t *)&TRISB, (uint16_t *)&ANSB,
             6, 6, 0, 6, (uint16_t *)&RPOR3);
    pin_init(&pin3, (uint16_t *)&PORTB, (uint16_t *)&TRISB, (uint16_t *)&ANSB,
             7, 7, 8, 7, (uint16_t *)&RPOR3);

    LIMIT_Y_FRONT_PIN = &pin0;
    LIMIT_Y_BACK_PIN = &pin1;
    RELAY_PIN = &pin2;

    pin_digitalIn(&D[LIMIT_X_LEFT_PIN]);
    pin_digitalIn(&D[LIMIT_X_RIGHT_PIN]);
    pin_digitalIn(LIMIT_Y_FRONT_PIN);
    pin_digitalIn(LIMIT_Y_BACK_PIN);
    pin_digitalOut(RELAY_PIN);
    pin_clear(RELAY_PIN);
}

// Joystick tracker
_POTENTIOMETER_TRACKER PotTracker;

int get_x() {
    int x = PotTracker.x_accumulator;
    // NOTE: The joystick is not centered.
    if (x < JOYSTICK_MID + JOYSTICK_TOL && x > JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is in the central deadband
        return 0;
    } else if (x >= JOYSTICK_MID + 2 * JOYSTICK_TOL) {
        return 10;
    } else if (x >= JOYSTICK_MID + JOYSTICK_TOL) {
        return 8;
    } else if (x <= JOYSTICK_MID - 3 * JOYSTICK_TOL) {
        return -10;
    } else if (x <= JOYSTICK_MID - JOYSTICK_TOL) {
        return -7;
    }
    return 0;  // Catch case
}

int get_y() {
    int y = PotTracker.y_accumulator;
    // NOTE: The joystick is not centered.
    if (y < JOYSTICK_MID + JOYSTICK_TOL && y > JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is in the central deadband
        return 0;
    } else if (y >= JOYSTICK_MID + 2 * JOYSTICK_TOL) {
        return 10;
    } else if (y >= JOYSTICK_MID + JOYSTICK_TOL) {
        return 8;
    } else if (y <= JOYSTICK_MID - 3 * JOYSTICK_TOL) {
        return -10;
    } else if (y <= JOYSTICK_MID - JOYSTICK_TOL) {
        return -8;
    }
    return 0;  // Catch case

}

uint16_t get_z() {
    uint16_t z = PotTracker.z_accumulator;
    uint16_t val = (z/67.0)*Z_STEP_SIZE; // be careful here - division in an interrupt!
    // printf("%u %u %u\n",z,(z>>6),val);
    return val;
}

void init_pot_tracking() {
    pin_analogIn(&A[X_PIN_IN]);
    pin_analogIn(&A[Y_PIN_IN]);
    pin_analogIn(&A[KNOB_PIN_IN]);
    PotTracker.x_accumulator = pin_read(&A[X_PIN_IN]) >> 6;
    PotTracker.y_accumulator = pin_read(&A[Y_PIN_IN]) >> 6;
    PotTracker.z_accumulator = pin_read(&A[KNOB_PIN_IN]) >> 6;
}

void track_pots(_TIMER *self) {
    int temp;
    // This complicated accumulator code is an attempt to calculate an
    // exponential moving average without float division, using only bitshifts
    // Idea taken from here: http://stackoverflow.com/a/10990656
    temp = (int) (pin_read(&A[X_PIN_IN]) >> 6);
    PotTracker.x_accumulator = (temp >> JOYSTICK_ALPHA) +
                               ((PotTracker.x_accumulator * ((1 << JOYSTICK_ALPHA) - 1)) >> JOYSTICK_ALPHA);

    temp = (int) (pin_read(&A[Y_PIN_IN]) >> 6);
    PotTracker.y_accumulator = (temp >> JOYSTICK_ALPHA) +
                               ((PotTracker.y_accumulator * ((1 << JOYSTICK_ALPHA) - 1)) >> JOYSTICK_ALPHA);

    temp = (int) (pin_read(&A[KNOB_PIN_IN]) >> 6);
    PotTracker.z_accumulator = (temp >> KNOB_ALPHA) +
                               ((PotTracker.z_accumulator * ((1 << KNOB_ALPHA) - 1)) >> KNOB_ALPHA);
}

#endif


#ifdef SCORE_PIC
// Coin tracker callback
void (*coin_callback)(void);

void init_coin_tracking(void (*callback)(void)) {
    coin_callback = callback;
    pin_digitalIn(&D[COIN_READ_PIN]);
    pin_digitalOut(&D[SCORE_START_STOP_PIN]);  //pin connecting this pic to the main pic

    // Coin interrupt
    INTCON2bits.INT0EP = 0; // interrupt 0 fires on pos edge
    IFS0bits.INT0IF = 0; // disable interrupt 0 flag
    IEC0bits.INT0IE = 1; // enable external interrupt 0
}

// Interrupt handler for INT0
void __attribute__((interrupt, auto_psv)) _INT0Interrupt(void) {
    IFS0bits.INT0IF = 0; // disable interrupt 0 flag
    printf("COIN\n");
    coin_callback();
}

// Ball tracker callback
void (*ball_callback)(int);

int last_win_value;
int last_lose_value;

void init_ball_tracking(void (*callback)(int)) {
    ball_callback = callback;
    pin_analogIn(&A[WIN_BALL_PIN]);
    pin_analogIn(&A[LOSE_BALL_PIN]);
    last_win_value = 0;
    last_lose_value = 0;
    timer_every(&timer3, 0.01, track_balls);
}

void track_balls() {
    int raw_win_value = pin_read(&A[WIN_BALL_PIN]) >> 6;
    int raw_lose_value = pin_read(&A[LOSE_BALL_PIN]) >> 6;
    if (raw_win_value > WIN_DIODE_LEVEL) {
        if (!last_win_value) {
            ball_callback(1);
            last_win_value = 1;
        }
    } else {
        last_win_value = 0;
    }

    if (raw_lose_value > LOSE_DIODE_LEVEL) {
        if (!last_lose_value) {
            ball_callback(0);
            last_lose_value = 1;
        }
    } else {
        last_lose_value = 0;
    }
}

void init_seven_segment(void) {
    spi_open(&spi1, &D[SPI_IN], &D[SPI_OUT], &D[SPI_CLK], 10000000.);
    timer_start(&timer2);
}

void display_elapsed_time(_TIMER *self) {
    uint16_t sampled_time = (uint16_t) timer_time(&timer2);
    spi_transfer(&spi1, sampled_time);
}

#endif
