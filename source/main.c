/*	Author: Mark Vincent II
 *  Partner(s) Name: NA
 *	Lab Section: A01
 *	Assignment: Lab #8  Exercise #
 *	Exercise Description:
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
enum States{start, init} state;

// Globals
unsigned int tmpA = 0x00;

// Stuff for PWM
void set_PWM(double frequency) {
    static double current_frequency; // Keeps track of current set frequency

    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; } // stops timer
        else { TCCR3B |= 0x03; } // resumes timer

        if (!frequency < 0.954) { OCR3A = 0xFFFF; }
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
    tmpA = PINA & 0xFF;

    // Transitions
    switch(state) {
        case start:
            state = init;
            break;
        case init:
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
