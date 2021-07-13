/*	Author: Mark Vincent II
 *  Partner(s) Name: NA
 *	Lab Section: A01
 *	Assignment: Lab #8  Exercise #3
 *	
 *  Youtube: https://youtu.be/zcbEohwC4GU
 *
 *  Exercise Description:
 *
 *  I am supposed to make my own 5 second song play on the speaker.
 *  
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
enum States{start, init, off, play, wait} state;
enum Pressed{pPress, pRelease} press;

// Globals
unsigned int tmpA = 0x00;

// Arrays for our song:
// notes to be played
double songN[] = {261.63, 261.63, 349.23, 392.00, 440.00, 440.00, 440.00,
        440.00, 349.23, 329.63, 349.23, 329.63}; // total = 12
// hold time
// 10 = 1 second (timer is at 100ms)
int songH[] = {8, 8, 2, 2, 3, 5, 5, 3, 2, 3, 3, 5};
// wait time
int songW[] = {8, 8, 5, 3, 3, 3, 3, 3, 3, 2, 5, 5};


int songIndex = 0; // index for current step of song.
int waitIndex = 0;
int holdIndex = 0;

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
            // when to play song or not
            if ((tmpA == 0x01) && (press == pPress)) {
                // do not play the song
                state = off;
                press = pPress;
            } else if ((tmpA == 0x01) && (press == pRelease)) {
                // play the song
                state = play;
                press = pPress;
            } else if ((tmpA == 0x00) && (press == pPress)) {
                // play the song
                state = play;
                press = pRelease;
            } else {
                // do not play the song
                state = off;
                press = pRelease;
            }
            break;
        case play:
            // hold note until time is over
            if (holdIndex >= songH[songIndex] - 1) {
                // move on to wait phase, reset holdIndex to 0.
                state = wait;
                holdIndex = 0;
            } else {
                // increment our holdIndex
                holdIndex = holdIndex + 1;
            }
            break;
        case wait:
            // wait silence until time is over
            if (waitIndex >= songW[songIndex] - 1) {
               // check if at the end of song
               if (songIndex == 11) {
                    // go to off state, reset songIndex and waitIndex
                    state = off;
                    songIndex = 0;
                    holdIndex = 0;
                    waitIndex = 0;
               } else {
                    // move on to play state, reset waitIndex to 0.
                    state = play;
                    waitIndex = 0;
                    // increment song note by 1
                    songIndex = songIndex + 1;
               }
            } else {
               // increment our waitIndex
               waitIndex = waitIndex + 1;
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
            PWM_on();
            press = pRelease;
            break;
        case off:
            set_PWM(0);
            break;
        case play:
            // play current note
            set_PWM(songN[songIndex]);
            break;
        case wait:
            // play no note
            set_PWM(0);
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
    
    // Sets timer to 100ms, and on
    TimerSet(100);
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
