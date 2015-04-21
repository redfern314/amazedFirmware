
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

#define X_MOTOR_TRISTATE        8
#define X_MOTOR_A_PIN           9
#define X_MOTOR_B_PIN           10
#define Y_MOTOR_TRISTATE        11
#define Y_MOTOR_A_PIN           12
#define Y_MOTOR_B_PIN           13
#define Z_MOTOR_PIN             3
#define PWM_FREQ                5000
#define motorXDutyCycle         1023
#define motorYDutyCycle         512

// TEMPORARY VALUE, WILL NEED TO BE MOVED TO FIT Y LIMIT SWITCHES
#define START_STOP_PIN          2  // Digital pin connected to Score PIC


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// --------------------------- VARIABLE DEFINITIONS --------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int8_t motorXDirection = 10;
int8_t prevMotorXDirection = 0;
int leftLimit = 0;  // Indicates whether left limit switch is flipped
int rightLimit = 0;  // Indicates whether right limit switch is flipped
int8_t motorYDirection = 10;
int8_t prevMotorYDirection = 0;
uint8_t vacuumOn = 0;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ----------------------- INTERRUPT CALLBACK FUNCTIONS-----------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int x_limit() {
    printf("Saw X limit switch, halting!\n");
    pin_clear(&D[X_MOTOR_A_PIN]);
    pin_clear(&D[X_MOTOR_B_PIN]);
}

int y_limit() {
    printf("Saw Y limit switch, halting!\n");
    pin_clear(&D[Y_MOTOR_A_PIN]);
    pin_clear(&D[Y_MOTOR_B_PIN]);
}

void setMotor() {
    // set the servo position based on z-axis pot
    int z = get_z();
    pin_write(&D[Z_MOTOR_PIN], z * Z_STEP_SIZE);

    // set the motor direction based on x-axis pot
    motorXDirection = get_x(); // update the direction from control_tools
    leftLimit = pin_read(&D[LIMIT_X_LEFT_PIN]);
    rightLimit = pin_read(&D[LIMIT_X_RIGHT_PIN]);

    if (motorXDirection != prevMotorXDirection || leftLimit || rightLimit) {
        oc_free(&oc2);
        if (motorXDirection > 0 && !leftLimit) {
            // Config PWM for IN1 (D10)
            oc_pwm(&oc2, &D[X_MOTOR_B_PIN], NULL, PWM_FREQ,
                   (abs(motorXDirection) * motorXDutyCycle / 10) << 6);
            // Set IN2 (D9) low
            pin_clear(&D[X_MOTOR_A_PIN]);
        } else if (motorXDirection < 0 && !rightLimit) {
            // Config PWM for IN2 (D9)
            oc_pwm(&oc2, &D[X_MOTOR_A_PIN], NULL, PWM_FREQ,
                   (abs(motorXDirection) * motorXDutyCycle / 10) << 6);
            // Set IN1 (D10) low
            pin_clear(&D[X_MOTOR_B_PIN]);
        } else {
            printf("STOPPP\n");
            pin_clear(&D[X_MOTOR_A_PIN]);
            pin_clear(&D[X_MOTOR_B_PIN]);
        }
        prevMotorXDirection = motorXDirection;
    }

    // // set the motor direction based on y-axis pot
    // motorYDirection = get_y(); // update the direction from control_tools

    // if (motorYDirection != prevMotorYDirection) {
    //     oc_free(&oc3);

    //     if (motorYDirection < 0) {
    //     // if (motorYDirection < 0 && !pin_read(&D[LIMIT_Y_FRONT_PIN])) {
    //         printf("Going FORWARD\n");
    //         // Config PWM for IN1 (D6)
    //         oc_pwm(&oc3, &D[Y_MOTOR_A_PIN], NULL, PWM_FREQ,
    //                (abs(motorYDirection) * motorYDirection / 10) << 6);
    //         // Set IN2 (D5) low
    //         pin_clear(&D[Y_MOTOR_B_PIN]);
    //     } else if (motorYDirection > 0) {
    //     // } else if (motorYDirection > 0 && !pin_read(&D[LIMIT_Y_BACK_PIN])) {
    //         printf("Going BACK\n");
    //         // Config PWM for IN2 (D5)
    //         oc_pwm(&oc3, &D[Y_MOTOR_B_PIN], NULL, PWM_FREQ,
    //                (abs(motorYDirection) * motorYDirection / 10) << 6);
    //         // Set IN1 (D6) low
    //         pin_clear(&D[Y_MOTOR_A_PIN]);
    //     } else {
    //         pin_clear(&D[Y_MOTOR_A_PIN]);
    //         pin_clear(&D[Y_MOTOR_B_PIN]);
    //     }
    //     prevMotorYDirection = motorYDirection;
    // }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------- ZERO AXES FUNCTION ---------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void zeroAxes() {
    // Go to hardcoded Z value
    pin_write(&D[Z_MOTOR_PIN], 2 * Z_STEP_SIZE);

    // Drive X to limit
    oc_pwm(&oc2, &D[X_MOTOR_A_PIN], NULL, PWM_FREQ, 400 << 6);
    pin_clear(&D[X_MOTOR_B_PIN]);
    while (!pin_read(&D[LIMIT_X_RIGHT_PIN])) {
        // Hang until done
    }
    pin_clear(&D[X_MOTOR_A_PIN]);

    // // Drive Y to limit
    // oc_pwm(&oc2, &D[Y_MOTOR_A_PIN], NULL, PWM_FREQ, 256 << 6);
    // pin_clear(&D[Y_MOTOR_B_PIN]);
    // while (!pin_read(&D[LIMIT_Y_FRONT_PIN])) {
    //     // Hang until done
    // }
    // pin_clear(&D[Y_MOTOR_A_PIN]);
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
    init_limit_tracking(&x_limit, &y_limit);

    setup_motor_shield();

    init_pot_tracking();
	timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);

    // Initialize z axis control
    pin_digitalOut(&D[Z_MOTOR_PIN]);
    oc_servo(&oc1, &D[Z_MOTOR_PIN], &timer5, 20E-3, 1E-3, 2E-3, 0); // start airflow at fully open

    // Initialize x axis control
    oc_pwm(&oc2, &D[X_MOTOR_A_PIN], NULL, PWM_FREQ, 0); // Motor PWM setup
    oc_pwm(&oc3, &D[Y_MOTOR_A_PIN], NULL, PWM_FREQ, 0); // Motor PWM setup
    timer_every(&timer2, 0.1, &setMotor); // Motor control interrupt

    // Debug timers
    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);

    pin_digitalIn(&D[START_STOP_PIN]);
    // pin_digitalOut(&D[RELAY_PIN]);

    printf("Starting up\n");
}

int16_t main(void) {
    setup();

    while (1) {       
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            if (pin_read(&D[START_STOP_PIN]) && !vacuumOn) {
                vacuumOn = 1;
                timer_disableInterrupt(&timer2);
                zeroAxes();
                timer_enableInterrupt(&timer2);
                // pin_set(&D[RELAY_PIN]);
            } else if (!pin_read(&D[START_STOP_PIN]) && vacuumOn) {
                vacuumOn = 0;
                // pin_clear(&D[RELAY_PIN]);
            }
            printf("Control inputs:\n");
            printf("\tX: %d\n", get_x());
            printf("\tY: %d\n", get_y());
            printf("\tZ: %d\n", get_z());
            printf("\tX LEFT LIMIT SWITCH: %d\n", leftLimit);
            printf("\tX RIGHT LIMIT SWITCH: %d\n", rightLimit);

        }
    }
}
