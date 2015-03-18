// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#ifndef _CONTROL_TOOLS_H_
#define _CONTROL_TOOLS_H_

#include <stdint.h>
#include "timer.h"

#define TRACK_COIN_FREQ         20  // 20 Hz. Too fast gives false positives
#define COIN_VOLTAGE_LEVEL      40  // Voltage level that indicates coin
#define COIN_READ_PIN           5   // Analog pin

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


#endif
