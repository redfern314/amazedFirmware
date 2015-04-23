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

pin_init(&pin0, (uint16_t *)&PORTG, (uint16_t *)&TRISG, (uint16_t *)NULL,
         9, -1, 8, 27, (uint16_t *)&RPOR13);
pin_init(&pin1, (uint16_t *)&PORTG, (uint16_t *)&TRISG, (uint16_t *)NULL,
         8, -1, 8, 19, (uint16_t *)&RPOR9);
pin_init(&pin2, (uint16_t *)&PORTB, (uint16_t *)&TRISB, (uint16_t *)&ANSB,
         6, 6, 0, 6, (uint16_t *)&RPOR3);
pin_init(&pin3, (uint16_t *)&PORTB, (uint16_t *)&TRISB, (uint16_t *)&ANSB,
         7, 7, 8, 7, (uint16_t *)&RPOR3);

_PIN *LIMIT_Y_FRONT_PIN, *LIMIT_Y_BACK_PIN;
LIMIT_Y_FRONT_PIN = &pin0;
LIMIT_Y_BACK_PIN = &pin1;

pin_digitalIn(&D[LIMIT_X_LEFT_PIN]);
pin_digitalIn(&D[LIMIT_X_RIGHT_PIN]);
pin_digitalIn(LIMIT_Y_FRONT_PIN);
pin_digitalIn(LIMIT_Y_BACK_PIN);


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

int get_z() {
    int z = PotTracker.z_accumulator;
    if (z <= 67) {
        return 0;
    } else if (z <= 134) {
        return 1;
    } else if (z <= 201) {
        return 2;
    } else if (z <= 268) {
        return 3;
    } else if (z <= 335) {
        return 4;
    } else if (z <= 402) {
        return 5;
    } else if (z <= 469) {
        return 6;
    } else if (z <= 536) {
        return 7;
    } else if (z <= 603) {
        return 8;
    } else if (z <= 670) {
        return 9;
    } else if (z <= 737) {
        return 10;
    } else if (z <= 804) {
        return 11;
    } else if (z <= 871) {
        return 12;
    } else {
        return 13;
    }
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

    // Configure an external interrupt on the coin input pin and for each of the 2 software limit switches
    __builtin_write_OSCCONL(OSCCON&0xBF);
    RPINR0bits.INT1R = 22; // equivalent to RPINR0 |= (22 << 8), sets INT1 to RP22 / D13
    __builtin_write_OSCCONL(OSCCON|0x40);

    // Coin interrupt
    INTCON2bits.INT1EP = 0; // interrupt 1 fires on pos edge
    IFS1bits.INT1IF = 0; // disable interrupt 1 flag
    IEC1bits.INT1IE = 1; // enable external interrupt 1
}

// Ball tracker callback
void (*ball_callback)(int);

void init_ball_tracking(void (*callback)(int)) {
    ball_callback = callback;
    pin_digitalIn(&D[WIN_BALL_PIN]);
    pin_digitalIn(&D[LOSE_BALL_PIN]);

    // Configure an interrupt on the ball input pin
    __builtin_write_OSCCONL(OSCCON&0xBF);
    RPINR0bits.INT1R = 22; // equivalent to RPINR0 |= (22 << 8), sets INT1 to RP22 / D13
    RPINR1bits.INT2R = 23; // equivalent to RPINR1 |= (3 << 8), sets INT2 to RP3 / D12
    __builtin_write_OSCCONL(OSCCON|0x40);

    INTCON2bits.INT1EP = 0; // interrupt 1 fires on neg edge
    IFS1bits.INT1IF = 0; // disable interrupt 1 flag
    IEC1bits.INT1IE = 1; // enable external interrupt 1

    INTCON2bits.INT2EP = 0; // interrupt 2 fires on neg edge
    IFS1bits.INT2IF = 0; // disable interrupt 2 flag
    IEC1bits.INT2IE = 1; // enable external interrupt 

}

// Interrupt handler for INT1
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void) {
    IFS1bits.INT1IF = 0; // disable interrupt flag
    ball_callback(1);
}

void init_seven_segment() {
    spi_open(&spi1, &D[SPI_IN], &D[SPI_OUT], &D[SPI_CLK], 10000000.);

    timer_start(&timer2);
}

void display_elapsed_time(_TIMER *self) {
    uint16_t sampled_time = (uint16_t) timer_time(&timer2);
    spi_transfer(&spi1,sampled_time);
}

#endif
