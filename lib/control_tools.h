// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#ifndef _CONTROL_TOOLS_H_
#define _CONTROL_TOOLS_H_

#include <stdint.h>
#include "timer.h"


// Coin tracker
#define COIN_READ_PIN       13   // Digital pin

// X-Y-Z tracker
#define X_PIN_IN            4    // Analog pin
#define Y_PIN_IN            3    // Analog pin
#define KNOB_PIN_IN         5    // Analog pin
#define TRACK_POT_FREQ      100  // 100 Hz.
#define JOYSTICK_MIN        805  // Analog voltage
#define JOYSTICK_MAX        940  // Analog voltage
#define JOYSTICK_MID        875  // Analog voltage
#define JOYSTICK_TOL        25   // Analog voltage
#define JOYSTICK_ALPHA      4    // Part of an exponential moving average
                                 // http://stackoverflow.com/a/10990656
                                 // NOTE: Defined here as 4 bits (16) instead
                                 // of 1/16 to avoid float division
#define KNOB_MIN            0    // Analog voltage
#define KNOB_MAX            940  // Analog voltage
#define KNOB_MID            470  // Analog voltage
#define KNOB_TOL            200  // Analog voltage
#define KNOB_ALPHA          2    // Part of an exponential moving average
                                 // http://stackoverflow.com/a/10990656
                                 // NOTE: Defined here as 2 bits (4_ instead of
                                 // 1/4 to avoid float division

void init_coin_tracking(void (*callback)(void));

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
