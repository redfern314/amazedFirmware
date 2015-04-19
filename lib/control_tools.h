// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#ifndef _CONTROL_TOOLS_H_
#define _CONTROL_TOOLS_H_

#include <stdint.h>
#include "timer.h"


// Coin tracker
#define COIN_READ_PIN       5    // Analog pin
#define TRACK_COIN_FREQ     20   // 20 Hz. Too fast gives false positives
#define COIN_VOLTAGE_LEVEL  40   // Voltage level that indicates coin

// Ball tracker
#define WIN_BALL_PIN        12   // Digital pin for interrupt
#define LOSE_BALL_PIN       13   // Digital pin for interrupt

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

#define Z_STEP_SIZE         4681 // (65535 - 1) / 14

// 7 segment
#define DIGIT1_PIN          0    // Pin for digit1
#define DIGIT2_PIN          1    // Pin for digit2
#define DIGIT3_PIN          2    // Pin for digit3
#define DIGIT4_PIN          3    // Pin for digit4
#define SEGMENTA_PIN        4
#define SEGMENTB_PIN        5   
#define SEGMENTC_PIN        6
#define SEGMENTD_PIN        7
#define SEGMENTE_PIN        8
#define SEGMENTF_PIN        9
#define SEGMENTG_PIN        10
#define SEGMENT_ON          10
#define SEGMENT_OFF         0

// Coin tracker
typedef struct _COIN_TRACKER {
    int voltage;
    int coin_in_slot;
    int new_coin;
    int total_coin_count;
} _COIN_TRACKER;

#ifndef SCORE_PIC
void init_game();
void init_coin_tracking(void (*callback)(void));
#endif

#ifdef SCORE_PIC
void init_ball_tracking(void (*callback)(int));
void display_elapsed_time();
void light_segments();
void clear_all();
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
