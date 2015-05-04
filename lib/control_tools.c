// Made by Eric Schneider and Forrest Bourke for Elecanisms 2015

#include <p24FJ128GB206.h>
#include <math.h>
#include <stdio.h>
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "oc.h"
#include "spi.h"
#include "ui.h"


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
    pin_clear(&D[SCORE_START_STOP_PIN]);

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

int last_win_value;
int last_lose_value;

// Ball tracker callback
void (*ball_callback)(int);

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

// Ball tracker callback
void (*vacuum_callback)(void);

void init_vacuum_tracking(void (*callback)(void)) {
    vacuum_callback = callback;
    pin_digitalIn(&D[VACUUM_READ_PIN]);

    // Configure an interrupt on the ball input pin
    __builtin_write_OSCCONL(OSCCON&0xBF);
    RPINR0bits.INT1R = 4; // sets INT1 to RP4 / D8
    __builtin_write_OSCCONL(OSCCON|0x40);

    INTCON2bits.INT1EP = 0; // interrupt 1 fires on neg edge
    IFS1bits.INT1IF = 0; // disable interrupt 1 flag
    IEC1bits.INT1IE = 1; // enable external interrupt 1
}

// Interrupt handler for INT1
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void) {
    IFS1bits.INT1IF = 0; // disable interrupt flag
    vacuum_callback();
}

void write_data(uint8_t addr, uint8_t data) {
    pin_clear(&D[SPI_LOAD]);
    spi_transfer(&spi2, addr);  
    spi_transfer(&spi2, data);
    pin_set(&D[SPI_LOAD]);
}

void clear_display(void) {
    for (uint8_t i = 1; i < 9; i++) {
        write_data(i, 0xf);
    }
}

void init_seven_segment(void) {
    pin_digitalOut(&D[SPI_LOAD]);
    spi_open(&spi2, &D[SPI_IN], &D[SPI_OUT], &D[SPI_CLK], 10000000.);

    write_data(0x0c,1);           //set mode to normal operation
    write_data(0x09,0xff);        //set Code B decode for digits 7–0
    write_data(0x0a,8);           //set intensity register
    write_data(0x0b,0x07);        //set to display digits 0 to 7

    clear_display();
}

void display_best_score(uint16_t score) {
    for (int i = 5; i < 9; i++) {
        uint8_t digit = score % 10;
        score = score / 10;

        printf("%i,%i\n", i,digit);

        write_data(i,digit);
    }
}

uint16_t time_passed = 0;

uint16_t get_time(void) {
    return time_passed;
}

void display_elapsed_time(_TIMER *self) {
    led_toggle(&led1);

    time_passed++;
    uint16_t sampled_time = time_passed;

    for (int i = 1; i < 5; i++) {
        uint8_t digit = sampled_time % 10;
        sampled_time = sampled_time / 10;

        write_data(i,digit);
    }
}

#endif
