
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ------------------------- PREPROCESSOR DIRECTIVES -------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "control_tools.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include "oc.h"
#include <stdio.h>

#define true                    1
#define false                   0

#define PWM_FREQ                5000
#define motorDutyCycle          (uint8_t)65535

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// --------------------------- VARIABLE DEFINITIONS --------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int8_t motorDirection = 10;
int8_t prevMotorDirection = 0;
uint8_t prevDutyCycle = 65535;
uint8_t coins_read = 0;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ----------------------- INTERRUPT CALLBACK FUNCTIONS-----------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void accept_coin() {
    printf("Saw a new coin!\n");
    coins_read++;
}

void setMotor() {
    motorDirection = get_x(); // update the direction from control_tools

    if (motorDirection != prevMotorDirection || motorDutyCycle != prevDutyCycle) {
        oc_free(&oc1);

        if (motorDirection == 10) {
            // Config PWM for IN1 (D6)
            oc_pwm(&oc1,&D[6],&timer3,PWM_FREQ,motorDutyCycle);

            // Set IN2 (D5) low
            pin_clear(&D[5]);
        } else if (motorDirection == -10) {
            // Config PWM for IN2 (D5)
            oc_pwm(&oc1,&D[5],&timer3,PWM_FREQ,motorDutyCycle);

            // Set IN1 (D6) low
            pin_clear(&D[6]);
        } else {
            pin_clear(&D[5]);
            pin_clear(&D[6]);
        }
        prevDutyCycle = motorDutyCycle;
        prevMotorDirection = motorDirection;
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ------------------------------ MAIN FUNCTIONS -----------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void setup() {
	init_clock();
    init_pin();
    init_timer();
    init_uart();
    init_ui();
    led_on(&led1); led_on(&led2); led_on(&led3);

    init_coin_tracking(&accept_coin);

    // while (coins_read == 0) { } // wait until a coin is inserted

    setup_motor_shield();

    init_pot_tracking();
	timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);    

    oc_pwm(&oc1, &D[6], &timer3, PWM_FREQ, 0); // Motor PWM setup
    timer_every(&timer2,0.1,&setMotor); // Motor control interrupt
    pin_analogIn(&A[5]); // Reading the joystick

    // Debug timers
    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);
}

int16_t main(void) {
    setup();

    while (1) {
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            // printf("Number of coins seen so far: %d\n", get_coins());
            printf("Control inputs:\n");
            printf("\tX: %d\n", get_x());
            printf("\tY: %d\n", get_y());
            printf("\tZ: %d\n", get_z());
            // printf("X: %d\tY: %d\n", get_x(), get_y());
        }
    }
}