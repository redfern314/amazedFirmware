// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#ifndef _CONTROL_TOOLS_H_
#define _CONTROL_TOOLS_H_

#include <stdint.h>
#include "timer.h"


// Coin tracker
#define COIN_READ_PIN       5    // Analog pin
#define TRACK_COIN_FREQ     20   // 20 Hz. Too fast gives false positives
#define COIN_VOLTAGE_LEVEL  40   // Voltage level that indicates coin

// X-Y-Z tracker
#define X_PIN_IN            4    // Analog pin
#define Y_PIN_IN            3    // Analog pin
#define KNOB_PIN_IN         2    // Analog pin
#define TRACK_POT_FREQ      100  // 100 Hz.
#define JOYSTICK_MIN        805  // Analog voltage
#define JOYSTICK_MAX        940  // Analog voltage
#define JOYSTICK_MID        875  // Analog voltage
#define JOYSTICK_TOL        8    // Analog voltage
#define JOYSTICK_ALPHA      4    // Part of an exponential moving average
                                 // http://stackoverflow.com/a/10990656
                                 // NOTE: Defined here as 4 bits (16) instead
                                 // of 1/16 to avoid float division
#define KNOB_MIN            1    // Analog voltage
#define KNOB_MAX            3    // Analog voltage
#define KNOB_MID            2    // Analog voltage
#define KNOB_TOL            1    // Analog voltage
#define KNOB_ALPHA          2    // Part of an exponential moving average
                                 // http://stackoverflow.com/a/10990656
                                 // NOTE: Defined here as 2 bits (4_ instead of
                                 // 1/4 to avoid float division


// Coin tracker
typedef struct _COIN_TRACKER {
    int voltage;
    int coin_in_slot;
    int new_coin;
    int total_coin_count;
} _COIN_TRACKER;

void init_coin_tracking();
int get_new_coin();
int get_coins();
void track_coins(_TIMER *self);


// Potentiometer tracker
typedef struct _POTENTIOMETER_TRACKER {
    int x_accumulator;
    int y_accumulator;
    int z_accumulator;
} _POTENTIOMETER_TRACKER;

void init_pot_tracking();
int get_x();
int get_y();
int get_z();
void track_pots();


#endif
