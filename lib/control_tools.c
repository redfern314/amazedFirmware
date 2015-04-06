// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#include <p24FJ128GB206.h>
#include <math.h>
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "oc.h"


#ifndef SCORE_PIC
// Coin tracker callback
void (*coin_callback)(void);

void init_coin_tracking(void (*callback)(void)) {
    coin_callback = callback;
    pin_digitalIn(&D[COIN_READ_PIN]);
    pin_digitalIn(&D[9]);
    pin_digitalIn(&D[10]);

    // Configure an external interrupt on the coin input pin and for each of the 2 software limit switches
    __builtin_write_OSCCONL(OSCCON&0xBF);
    RPINR0bits.INT1R = 22; // equivalent to RPINR0 |= (22 << 8), sets INT1 to RP22 / D13
    RPINR1bits.INT2R = 3; // equivalent to RPINR1 |= (3 << 8), sets INT2 to RP3 / D9
    __builtin_write_OSCCONL(OSCCON|0x40);

    // Coin interrupt
    INTCON2bits.INT1EP = 0; // interrupt 1 fires on pos edge
    IFS1bits.INT1IF = 0; // disable interrupt 1 flag
    IEC1bits.INT1IE = 1; // enable external interrupt 1

    // Left limit switch interrupt
    INTCON2bits.INT2EP = 0; // interrupt 2 fires on pos edge
    IFS1bits.INT2IF = 0; // disable interrupt 2 flag
    IEC1bits.INT2IE = 1; // enable external interrupt 2

    // Right limit switch interrupt
    INTCON2bits.INT0EP = 0; // interrupt 0 fires on pos edge
    IFS0bits.INT0IF = 0; // disable interrupt 0 flag
    IEC0bits.INT0IE = 1; // enable external interrupt 0
}

// Interrupt handler for INT1
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void) {
    IFS1bits.INT1IF = 0; // disable interrupt 1 flag
    coin_callback();
}

// Interrupt handler for INT2
void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void) {
    IFS1bits.INT2IF = 0; // disable interrupt 2 flag
    printf("OH GOD LIMIT SWITCHES 2\r\n");
}

// Interrupt handler for INT0
void __attribute__((interrupt, auto_psv)) _INT0Interrupt(void) {
    IFS0bits.INT0IF = 0; // disable interrupt 0 flag
    printf("OH GOD LIMIT SWITCHES 0\r\n");
}

#endif


#ifdef SCORE_PIC
// Ball tracker callback
void (*ball_callback)(int);

void init_ball_tracking(void (*callback)(int)) {
    ball_callback = callback;
    pin_digitalIn(&D[WIN_BALL_PIN]);
    pin_digitalIn(&D[LOSE_BALL_PIN]);

    // Configure an interrupt on the coin input pin
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

// Interrupt handler for INT2
void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void) {
    IFS1bits.INT2IF = 0; // disable interrupt 2 flag
    ball_callback(0);
}

#endif


// Joystick tracker
_POTENTIOMETER_TRACKER PotTracker;

int get_x() {
    int x = PotTracker.x_accumulator;
    if (x < JOYSTICK_MID + JOYSTICK_TOL && x > JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is in the central deadband
        return 0;
    } else if (x >= JOYSTICK_MID + JOYSTICK_TOL) {
        // Joystick is full on (consider making variable map)
        return 10;
    } else if (x <= JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is full off (consider making variable map)
        return -10;
    }
    return 0;  // Catch case
}

int get_y() {
    int y = PotTracker.y_accumulator;
    if (y < JOYSTICK_MID + JOYSTICK_TOL && y > JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is in the central deadband
        return 0;
    } else if (y >= JOYSTICK_MID + JOYSTICK_TOL) {
        // Joystick is full on (consider making variable map)
        return 10;
    } else if (y <= JOYSTICK_MID - JOYSTICK_TOL) {
        // Joystick is full off (consider making variable map)
        return -10;
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
    } else if (z <= KNOB_MAX) {
        return 13;
    }
    // if (z < KNOB_MID + KNOB_TOL && z > KNOB_MID - KNOB_TOL) {
    //     // Knob is in the central deadband
    //     return 0;
    // } else if (z >= KNOB_MID + KNOB_TOL) {
    //     // Knob is full on (consider making variable map)
    //     return 10;
    // } else if (z <= KNOB_MID - KNOB_TOL) {
    //     // Joystick is full off (consider making variable map)
    //     return -10;
    // }
    return 7;  // Catch case
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