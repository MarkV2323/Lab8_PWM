/*	Author: Mark Vincent II
 *  Partner(s) Name: NA
 *	Lab Section: A01
 *	Assignment: Lab #8  Exercise #2
 *  Youtube: https://youtu.be/MfVMXCXNUEY
 *	Exercise Description:
 *
 *  Create a system that allows for scaling of the notes.
 *  Button 1: toggle sound on or off A0
 *  Button 2: scale down 1 note A1
 *  Button 3: scale up 1 note A2
 *  
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif

// States
enum States{start, init, off, on} state;

// Checks for presses
enum Pressed{pOff, pA0, pA1, pA2} press;


// Globals
unsigned int tmpA = 0x00;
unsigned int note = 0;

// Stuff for PWM
void set_PWM(double frequency) {
    static double current_frequency; // Keeps track of current set frequency

    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; } // stops timer
        else { TCCR3B |= 0x03; } // resumes timer

        if (frequency < 0.954) { OCR3A = 0xFFFF; }
        else if (frequency > 31250) { OCR3A = 0x0000; }
        else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }

        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

// Tick Function
void tick() {
    
    // READ INPUT
    tmpA = ~PINA & 0xFF;

    // Transitions
    switch(state) {
        case start:
            state = init;
            break;
        case init:
            state = off;
            break;
        case off:
            // checks for next state
            if ((tmpA == 0x01) && (press == pOff)) {
                // move to on
                state = on;
                press = pA0;
            } else if (tmpA == 0x00) {
                // stay in off
                state = off;
                press = pOff;
            }
            break;
        case on:
            // checks for next state
            if ((tmpA == 0x01) && (press == pOff)) {
                // move to off
                state = off;
                press = pA0;
            } else if (tmpA == 0x01 && press == pA0){
                // stay in on
                state = on;
            } else if (tmpA == 0x00) {
                state = on;
                press = pOff;
            }
            
            // checks for increment or decrement if STILL in on state.
            if (state == on && tmpA == 0x02 && press != pA1) {
                // does a pA1 action (scale down by 1)
                if (note > 0) {
                    note = note - 1;
                }
                press = pA1;
            } else if (state == on && tmpA == 0x04 && press != pA2) {
                // does a pA2 action (scale up by 1)
                if (note < 7) {
                    note = note + 1;
                }
                press = pA2;
            }
            break;    
        default:
            state = start;
            break;
    }

    // Actions
    switch(state) {
        case start:
            break;
        case init:
            // turn on speaker
            PWM_on();
            // setup press
            press = pOff;
            break;
        case off:
            // set speaker to no noise
            set_PWM(0);
            break;
        case on:
            // plays frequency
            switch(note) {
                case 0:
                    set_PWM(261.63);
                    break;
                case 1:
                    set_PWM(293.66);
                    break;
                case 2:
                    set_PWM(329.63);
                    break;
                case 3:
                    set_PWM(349.23);
                    break;
                case 4:
                    set_PWM(392.00);
                    break;
                case 5:
                    set_PWM(440.00);
                    break;
                case 6:
                    set_PWM(493.88);
                    break;
                case 7:
                    set_PWM(523.25);
                    break;
                default:
                    break;
            }
            // If nothing is being pressed.
            if (tmpA == 0x00) {
                press = pOff;
            }
            break;
        default:
            break;
    }

}

// Main Function using timer.h header for timer.
int main(void) {
    
    // PORTS
    DDRA = 0x00; PORTA = 0xFF; // PortA as input
    DDRB = 0xFF; PORTB = 0x00; // PortB as output
    
    // Sets timer to 50ms, and on
    TimerSet(50);
    TimerOn();

    // init state
    state = start;
    
    // primary loop
    while (1) {
        
        // tick
        tick();

        // wait
        while (!TimerFlag);

        TimerFlag = 0;
    }

    return 1;
}
