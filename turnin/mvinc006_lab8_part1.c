/*	Author: Mark Vincent II
 *  Partner(s) Name: NA
 *	Lab Section: A01
 *	Assignment: Lab #8  Exercise #1
 *	
 *  Youtube Link: https://youtu.be/pS_sSxK9jes 
 *  Exercise Description:
 *
 *  Design a system that uses three buttons (A0, A1, A2) to select one of three
 *  tones to be generated on the speaker.
 *  Specs:
 *  Use tones C_4 D_4 and E_4 (mapped to A0, A1, and A2)
 *  When a button is pressed AND held, the tone is applied to the speaker.
 *  When more than one button is pressed, the speaker is slient.
 *  When no buttons are pressed, the speaker remains silent.
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
enum States{start, init, off, onA0, onA1, onA2} state;

// Flags
enum Pressed{poff, pA0, pA1, pA2} press;

// Globals
unsigned int tmpA = 0x00;

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
            // Checks which on state to move to
            if (tmpA == 0x01) {
                // onA0 pressed
                state = onA0;
            } else if (tmpA == 0x02) {
                // onA1 pressed
                state = onA1;
            } else if (tmpA == 0x04) {
                // onA2 pressed
                state = onA2;
            } else {
                // stay in off
                state = off;
            }
            break;

        case onA0:
            // Checks which state to move to
            if (tmpA == 0x01) {
                // stay in onA0
                state = onA0;
            } else {
                // move to off and set flag to off
                state = off;
                press = poff;
            }
            break;

        case onA1:
            // Checks which state to move to
            if (tmpA == 0x02) {
                // stay in onA1
                state = onA1;
            } else {
                // move to off and set flag to off
                state = off;
                press = poff;
            }
            break;

        case onA2:
           // Checks which state to move to
            if (tmpA == 0x04) {
                // stay in onA2
                state = onA2;
            } else {
                // move to off and set flag to off
                state = off;
                press = poff;
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
            press = poff;
            break;
        
        case off:
            // set press to off
            press = poff;
            // turn tones to off
            set_PWM(0);
            break;

        case onA0:
            // checks if being held
            if (press == pA0) {
                // Begin outputting tone
                // C_4 = 261.63
                set_PWM(261.63);
                //PWM_on();
            } else {
                // Set press to pA0
                press = pA0;
            }
            break;

        case onA1:
            // checks if being held
            if (press == pA1) {
                // Begin outputting tone
                // D_4 = 293.66
                set_PWM(293.66);
                //PWM_on();
            } else {
                // Set press to pA0
                press = pA1;
            }

            break;

        case onA2:
            // checks if being held
            if (press == pA2) {
                // Begin outputting tone
                // E_4 = 329.63
                set_PWM(329.63);
                //PWM_on();
            } else {
                // Set press to pA0
                press = pA2;
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
    
    PWM_on();

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
