
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
#define motorYDutyCycle         860

// TEMPORARY VALUE, WILL NEED TO BE MOVED TO FIT Y LIMIT SWITCHES
#define START_STOP_PIN          2  // Digital pin connected to Score PIC


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// --------------------------- VARIABLE DEFINITIONS --------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int8_t motorXDirection = 0;
int8_t prevMotorXDirection = 0;
uint8_t leftLimit = 0;  // Indicates whether left limit switch is flipped
uint8_t rightLimit = 0;  // Indicates whether right limit switch is flipped
int8_t motorYDirection = 0;
int8_t prevMotorYDirection = 0;
uint8_t frontLimit = 0;  // Indicates whether front limit switch is flipped
uint8_t backLimit = 0;  // Indicates whether back limit switch is flipped
uint8_t vacuumOn = 0;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ----------------------- INTERRUPT CALLBACK FUNCTIONS-----------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void setMotor() {
    // set the servo position based on z-axis pot
    int z = get_z();
    pin_write(&D[Z_MOTOR_PIN], z * Z_STEP_SIZE);

    // set the motor direction based on x-axis pot
    motorXDirection = get_x(); // update the direction from control_tools
    leftLimit = pin_read(&D[LIMIT_X_LEFT_PIN]);
    rightLimit = pin_read(&D[LIMIT_X_RIGHT_PIN]);

    if (motorXDirection != prevMotorXDirection || leftLimit || rightLimit) {
        if (motorXDirection > 0 && !leftLimit) {
            pin_set(&D[X_MOTOR_B_PIN]);
            pin_clear(&D[X_MOTOR_A_PIN]);
        } else if (motorXDirection < 0 && !rightLimit) {
            pin_set(&D[X_MOTOR_A_PIN]);
            pin_clear(&D[X_MOTOR_B_PIN]);
        } else {
            pin_clear(&D[X_MOTOR_A_PIN]);
            pin_clear(&D[X_MOTOR_B_PIN]);
        }
        pin_write(&D[X_MOTOR_TRISTATE], (abs(motorXDirection) * motorXDutyCycle / 10) << 6);
        prevMotorXDirection = motorXDirection;
    }

    // // set the motor direction based on y-axis pot
    motorYDirection = get_y(); // update the direction from control_tools
    frontLimit = pin_read(LIMIT_Y_FRONT_PIN);
    backLimit = pin_read(LIMIT_Y_BACK_PIN);

    if (motorXDirection != prevMotorXDirection || leftLimit || rightLimit) {
        if (motorYDirection < 0 && !frontLimit) {
            pin_set(&D[Y_MOTOR_A_PIN]);
            pin_clear(&D[Y_MOTOR_B_PIN]);
        } else if (motorYDirection > 0 && !backLimit) {
            pin_set(&D[Y_MOTOR_B_PIN]);
            pin_clear(&D[Y_MOTOR_A_PIN]);
        } else {
            pin_clear(&D[Y_MOTOR_A_PIN]);
            pin_clear(&D[Y_MOTOR_B_PIN]);
        }
        pin_write(&D[Y_MOTOR_TRISTATE], (abs(motorYDirection) * motorYDutyCycle / 10) << 6);
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
    pin_write(&D[Z_MOTOR_PIN], 2 * Z_STEP_SIZE);

    // Drive X to limit
    pin_write(&D[X_MOTOR_TRISTATE], 614 << 6);
    pin_set(&D[X_MOTOR_A_PIN]);
    pin_clear(&D[X_MOTOR_B_PIN]);
    while (!pin_read(&D[LIMIT_X_RIGHT_PIN])) {
        // Hang until done
    }
    pin_clear(&D[X_MOTOR_A_PIN]);

    // // Drive Y to limit
    pin_write(&D[Y_MOTOR_TRISTATE], 614 << 6);
    pin_clear(&D[Y_MOTOR_A_PIN]);
    pin_set(&D[Y_MOTOR_B_PIN]);
    while (!pin_read(LIMIT_Y_FRONT_PIN) {
        // Hang until done
    }
    pin_clear(&D[Y_MOTOR_B_PIN]);
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
    setup_motor_shield();

    init_pot_tracking();
	timer_every(&timer3, 1.0 / TRACK_POT_FREQ, track_pots);

    // Initialize Z axis control
    pin_digitalOut(&D[Z_MOTOR_PIN]);
    oc_servo(&oc1, &D[Z_MOTOR_PIN], &timer5, 20E-3, 1E-3, 2E-3, 0); // start airflow at fully open

    // Initialize X and Y axis control
    oc_pwm(&oc2, &D[X_MOTOR_TRISTATE], NULL, PWM_FREQ, 0); // Motor PWM setup
    oc_pwm(&oc3, &D[Y_MOTOR_TRISTATE], NULL, PWM_FREQ, 0); // Motor PWM setup
    timer_every(&timer2, 0.01, &setMotor); // Motor control interrupt

    // Debug timers
    timer_setPeriod(&timer1, 0.1);
    timer_start(&timer1);

    pin_digitalIn(&D[START_STOP_PIN]);
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
            } else if (!pin_read(&D[START_STOP_PIN]) && vacuumOn) {
                vacuumOn = 0;
            }
            printf("Control inputs:\n");
            printf("\tX: %d\n", get_x());
            printf("\tY: %d\n", get_y());
            printf("\tZ: %d\n", get_z());
            // printf("\tX left limit switch: %d\n", leftLimit);
            // printf("\tX right limit switch: %d\n", rightLimit);
            // printf("\tY front limit switch: %d\n", frontLimit);
            // printf("\tY back limit switch: %d\n", backLimit);

        }
    }
}
