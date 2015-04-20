
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

#define X_MOTOR_A               &D[9]
#define X_MOTOR_B               &D[10]
#define Y_MOTOR_A               &D[12]
#define Y_MOTOR_B               &D[13]
#define Z_MOTOR                 &D[3]
#define PWM_FREQ                5000
#define motorXDutyCycle         1023
#define motorYDutyCycle         512

#define START_STOP_PIN          6  // Digital pin connected to Score PIC
// #define START_STOP_PIN          x  // Digital pin connected to Score PIC
#define RELAY_PIN               5  // Pin connected relay
// #define RELAY_PIN               x  // Pin connected relay

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// --------------------------- VARIABLE DEFINITIONS --------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int8_t motorXDirection = 10;
int8_t prevMotorXDirection = 0;
int8_t motorYDirection = 10;
int8_t prevMotorYDirection = 0;
uint8_t coins_read = 0;
uint8_t vacuum_on = 0;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ----------------------- INTERRUPT CALLBACK FUNCTIONS-----------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void accept_coin() {
    printf("Saw a new coin!\n");
    coins_read++;
}

int x_limit() {
    pin_clear(X_MOTOR_A);
    pin_clear(X_MOTOR_B);
}

int y_limit() {
    pin_clear(Y_MOTOR_A);
    pin_clear(Y_MOTOR_B);
}

void setMotor() {
    // set the servo position based on z-axis pot
    int z = get_z();
    pin_write(Z_MOTOR, z * Z_STEP_SIZE);

    // set the motor direction based on x-axis pot
    motorXDirection = get_x(); // update the direction from control_tools

    if (motorXDirection != prevMotorXDirection) {
        oc_free(&oc2);
        if (motorXDirection < 0 && !pin_read(&D[LIMIT_X_LEFT_PIN])) {
            // Config PWM for IN1 (D6)
            oc_pwm(&oc2, X_MOTOR_A, NULL, PWM_FREQ,
                   (abs(motorXDirection) * motorXDutyCycle / 10) << 6);
            // Set IN2 (D5) low
            pin_clear(X_MOTOR_B);
        } else if (motorXDirection > 0 && !pin_read(&D[LIMIT_X_RIGHT_PIN])) {
            // Config PWM for IN2 (D5)
            oc_pwm(&oc2, X_MOTOR_B, NULL, PWM_FREQ,
                   (abs(motorXDirection) * motorXDutyCycle / 10) << 6);
            // Set IN1 (D6) low
            pin_clear(X_MOTOR_A);
        } else {
            pin_clear(X_MOTOR_A);
            pin_clear(X_MOTOR_B);
        }
        prevMotorXDirection = motorXDirection;
    }

    // set the motor direction based on y-axis pot
    motorYDirection = get_y(); // update the direction from control_tools

    if (motorYDirection != prevMotorYDirection) {
        oc_free(&oc3);

        if (motorYDirection < 0 && !pin_read(&D[LIMIT_Y_BACK_PIN])) {
            // Config PWM for IN1 (D6)
            oc_pwm(&oc3, Y_MOTOR_A, NULL, PWM_FREQ,
                   (abs(motorYDirection) * motorYDirection / 10) << 6);
            // Set IN2 (D5) low
            pin_clear(Y_MOTOR_B);
        } else if (motorYDirection > 0 && !pin_read(&D[LIMIT_Y_FRONT_PIN])) {
            // Config PWM for IN2 (D5)
            oc_pwm(&oc3, Y_MOTOR_B, NULL, PWM_FREQ,
                   (abs(motorYDirection) * motorYDirection / 10) << 6);
            // Set IN1 (D6) low
            pin_clear(Y_MOTOR_A);
        } else {
            pin_clear(Y_MOTOR_A);
            pin_clear(Y_MOTOR_B);
        }
        prevMotorYDirection = motorYDirection;
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------- ZERO AXES FUNCTION ---------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void zeroAxes() {
    // Go to hardcoded Z value
    pin_write(Z_MOTOR, 2 * Z_STEP_SIZE);

    // Drive X to limit
    oc_pwm(&oc2, X_MOTOR_A, NULL, PWM_FREQ, 256 << 6);
    pin_clear(X_MOTOR_B);
    while (!pin_read(&D[LIMIT_X_RIGHT_PIN])) {
        // Hang until done
    }
    pin_clear(X_MOTOR_A);

    // Drive Y to limit
    oc_pwm(&oc2, Y_MOTOR_A, NULL, PWM_FREQ, 256 << 6);
    pin_clear(Y_MOTOR_B);
    while (!pin_read(&D[LIMIT_Y_FRONT_PIN])) {
        // Hang until done
    }
    pin_clear(Y_MOTOR_A);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ------------------------------ MAIN FUNCTIONS -----------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void setup() {
	init_clock();
    init_oc();
    init_pin();
    init_timer();
    init_uart();
    init_ui();

    led_on(&led1); led_on(&led3);
    init_coin_and_limit_tracking(&accept_coin, &x_limit, &y_limit);

    setup_motor_shield();

    init_pot_tracking();
	timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);

    // Initialize z axis control
    pin_digitalOut(Z_MOTOR);
    oc_servo(&oc1, Z_MOTOR, &timer5, 20E-3, 1E-3, 2E-3, 0); // start airflow at fully open

    // Initialize x axis control
    oc_pwm(&oc2, X_MOTOR_A, NULL, PWM_FREQ, 0); // Motor PWM setup
    oc_pwm(&oc3, Y_MOTOR_A, NULL, PWM_FREQ, 0); // Motor PWM setup
    timer_every(&timer2, 0.1, &setMotor); // Motor control interrupt

    // Debug timers
    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);

    pin_digitalIn(&D[START_STOP_PIN]);
    pin_digitalOut(&D[RELAY_PIN]);

    printf("Starting up\n");
}

int16_t main(void) {
    setup();

    while (1) {       
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            if (pin_read(&D[START_STOP_PIN]) && !vacuum_on) {
                vacuum_on = 1;
                timer_disableInterrupt(&timer2);
                zeroAxes();
                timer_enableInterrupt(&timer2);
                pin_set(&D[RELAY_PIN]);
            } else if (!pin_read(&D[START_STOP_PIN]) && vacuum_on) {
                vacuum_on = 0;
                pin_clear(&D[RELAY_PIN]);
            }
            // printf("Number of coins seen so far: %d\n", get_coins());
            printf("Control inputs:\n");
            printf("\tX: %d\n", get_x());
            printf("\tY: %d\n", get_y());
            printf("\tZ: %d\n", get_z());
            printf("X: %d\tY: %d\n", get_x(), get_y());
        }
    }
}
