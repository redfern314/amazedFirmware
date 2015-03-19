// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#include <p24FJ128GB206.h>
#include <math.h>
#include "common.h"
#include "control_tools.h"
#include "pin.h"


// Coin tracker
_COIN_TRACKER CoinTracker;

int get_new_coin() {
    if (CoinTracker.new_coin > 0) {
        CoinTracker.new_coin -= 1;
        return 1;
    }
    return 0;
}

int get_coins() {
    return CoinTracker.total_coin_count;
}

void init_coin_tracking() {
    CoinTracker.coin_in_slot = 0;
    CoinTracker.new_coin = 0;
    CoinTracker.total_coin_count = 0;
    pin_analogIn(&A[COIN_READ_PIN]);
    CoinTracker.voltage = pin_read(&A[COIN_READ_PIN]) >> 6;
}

void track_coins(_TIMER *self) {
    // Get voltage output by coin tracker
    CoinTracker.voltage = (int) (pin_read(&A[COIN_READ_PIN]) >> 6);

    if (CoinTracker.voltage > COIN_VOLTAGE_LEVEL) {
        CoinTracker.coin_in_slot = 1;
    } else if (CoinTracker.voltage < COIN_VOLTAGE_LEVEL) {
        if (CoinTracker.coin_in_slot == 1) {
            CoinTracker.new_coin += 1;
            CoinTracker.total_coin_count += 1;
        }
        CoinTracker.coin_in_slot = 0;
    }
}


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
    if (z < KNOB_MID + KNOB_TOL && z > KNOB_MID - KNOB_TOL) {
        // Knob is in the central deadband
        return 0;
    } else if (z >= KNOB_MID + KNOB_TOL) {
        // Knob is full on (consider making variable map)
        return 10;
    } else if (z <= KNOB_MID - KNOB_TOL) {
        // Joystick is full off (consider making variable map)
        return -10;
    }
    return 0;  // Catch case
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
