// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#include <p24FJ128GB206.h>
#include <math.h>
#include "common.h"
#include "control_tools.h"
#include "pin.h"

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
