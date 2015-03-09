
#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "ui.h"
#include "timer.h"
#include "pin.h"
#include "oc.h"
#include "uart.h"
#include <stdio.h>

#define true                    1
#define false                   0

#define PWM_FREQ                5000
#define motorDutyCycle          65535

#define JOYSTICK_MIN            805
#define JOYSTICK_MAX            940
#define JOYSTICK_MID            875
#define JOYSTICK_TOL            7

int motorDirection = 1;
int prevMotorDirection = 0;
int prevDutyCycle = 65535;


uint16_t sum = 0;
uint8_t sumnum = 0;

void readJoystick() {
    sumnum++;
    uint16_t data = pin_read(&A[5])>>6; // get the ADC reading
    sum += data;

    if (sumnum >= 8) {
        uint16_t avg = sum>>3;
        if (avg > (JOYSTICK_MID + JOYSTICK_TOL)) {
            motorDirection = -1;
            printf("Driving left\r\n");
        } else if (avg < (JOYSTICK_MID - JOYSTICK_TOL)) {
            motorDirection = 1;
            printf("Driving right\r\n");
        } else {
            motorDirection = 0;
            printf("Not driving\r\n");
        }
        // printf("%u\r\n", avg);
        sum = 0;
        sumnum = 0;
    }
}

void setMotor() {
    if (motorDirection != prevMotorDirection || motorDutyCycle != prevDutyCycle) {
        oc_free(&oc1);

        if (motorDirection == 1) {
            // Config PWM for IN1 (D6)
            oc_pwm(&oc1,&D[6],&timer3,PWM_FREQ,motorDutyCycle);

            // Set IN2 (D5) low
            pin_clear(&D[5]);
        } else if (motorDirection == -1) {
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

int16_t main(void) {
    init_clock();
    init_ui();
    init_timer();
    init_pin();
    init_oc();
    init_uart();

    led_on(&led3);

    // Motor GPIO pins setup
    pin_digitalOut(&D[4]);
    pin_set(&D[4]);// Set EN high

    pin_digitalOut(&D[2]);
    pin_set(&D[2]); // Set D2 high

    pin_digitalOut(&D[3]);
    pin_clear(&D[3]); // Set D1 low

    pin_digitalOut(&D[6]); 
    
    pin_digitalOut(&D[5]); 
    pin_clear(&D[5]); // Set IN2 low

    // Ignore SLEW (D7)
    // Ignore INV (D8)

    pin_digitalIn(&D[1]); // Status flag (D1)
    pin_analogIn(&A[0]); // Current sensor from 10mOhm resistor, gain * 10 (A0)
    pin_analogIn(&A[1]); // Back EMF from motor (A1)

    // PWM setup
    oc_pwm(&oc1,&D[6],&timer3,PWM_FREQ,motorDutyCycle);

    pin_analogIn(&A[5]); // Reading of the joystick

    // Interrupt setup
    timer_every(&timer1,0.01,&readJoystick);
    timer_every(&timer2,0.1,&setMotor);

    while (1) {
        
    }
}