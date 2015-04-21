// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#ifndef _CONTROL_TOOLS_H_
#define _CONTROL_TOOLS_H_

#include <stdint.h>
#include "timer.h"


// Coin tracker
#define COIN_READ_PIN       5    // Analog pin
#define TRACK_COIN_FREQ     20   // 20 Hz. Too fast gives false positives
#define COIN_VOLTAGE_LEVEL  40   // Voltage level that indicates coin

// Limit switch input pins
#define LIMIT_X_LEFT_PIN    0
#define LIMIT_X_RIGHT_PIN   1
// #define LIMIT_Y_BACK_PIN    2
// #define LIMIT_Y_FRONT_PIN   3

// Ball tracker
#define WIN_BALL_PIN        12   // Digital pin for interrupt
#define LOSE_BALL_PIN       13   // Digital pin for interrupt

// X-Y-Z tracker
#define X_PIN_IN            2    // Analog pin
#define Y_PIN_IN            1    // Analog pin
#define KNOB_PIN_IN         0    // Analog pin
#define TRACK_POT_FREQ      100  // 100 Hz.
#define JOYSTICK_MIN        800  // Analog voltage
#define JOYSTICK_MAX        900  // Analog voltage
#define JOYSTICK_MID        865  // Analog voltage
#define JOYSTICK_TOL        10   // Analog voltage
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

#define Z_STEP_SIZE         4681 // (65535 - 1) / 14


// Coin tracker
typedef struct _COIN_TRACKER {
    int voltage;
    int coin_in_slot;
    int new_coin;
    int total_coin_count;
} _COIN_TRACKER;

#ifndef SCORE_PIC
void init_coin_tracking(void (*callback)(void));
#endif

#ifdef SCORE_PIC
void init_ball_tracking(void (*callback)(int));
#endif

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
