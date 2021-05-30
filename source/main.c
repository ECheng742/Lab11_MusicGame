/* Author: Ellie Cheng - echen111
 * Lab Section: 022
 * Assignment: Lab 11 
 * Exercise Description: [optional - include for your own benefit]
 * Music Game
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: Youtube URL> 
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "../header/bit.h"
#include "../header/timer.h"
#include "../header/keypad.h"
#include "../header/scheduler.h"
#endif

//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Demo_States {shift};
int Demo_Tick(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char row = 0xFE;  	// Row(s) displaying pattern. 
							            // 0: display pattern on row
							            // 1: do NOT display pattern on row
	// Transitions
	switch (state) {
		case shift:	
            break;
		default:	
            state = shift;
			break;
	}	
	// Actions
	switch (state) {
        case shift:	
            if (row == 0xEF && pattern == 0x01) { // Reset demo 
				pattern = 0x80;		    
				row = 0xFE;
			} else if (pattern == 0x01) { // Move LED to start of next row
				pattern = 0x80;
				row = (row << 1) | 0x01;
			} else { // Shift LED one spot to the right on current row
				pattern >>= 1;
			}
			break;
		default:
	        break;
	}
	PORTC = pattern;	// Pattern to display
	PORTD = row;		// Row(s) displaying pattern	
	return state;
}

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; }
        else { TCCR3B |= 0x03; }

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

// TONE SM: When a button is pressed, a corresponding note is played
// If multiple buttons pressed, nothing played
enum TONE_States { TONE_SMStart, TONE_wait, TONE_note } TONE_State;

double noteFrequency = 0x00;

int ToneSMTick(int state) {
    set_PWM(392.00);
    unsigned char button = ~PINB & 0x1F;

    switch(state) {
        case TONE_SMStart:
            state = TONE_wait;
            break;

        case TONE_wait:
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                state = TONE_note;
            }
            break;

        case TONE_note:
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                state = TONE_note;
            }
            break;

        default:
            state = TONE_SMStart;
            break;
    } 

    switch(state) {
        case TONE_note:
            if (button == 0x01) { // Note C - 261.63
                noteFrequency = 261.63;
            }
            else if (button == 0x02) { // Note D - 293.66
                noteFrequency = 293.66;
            }
            else if (button == 0x04) { // Note E - 329.63
                noteFrequency = 329.63;
            }
            else if (button == 0x08) { // Note F - 349.23
                noteFrequency = 349.23;
            }
            else if (button == 0x10) { // Note G - 392.00
                noteFrequency = 392.00;
            }
            else { 
                noteFrequency = 0x00;
            }
            break;

        default:
            break;
            
        set_PWM(392.00);
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xE0; PORTB = 0x1F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */

    static task task2;
    task *tasks[] = { &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    // task1.state = start;
    // task1.period = 100;
    // task1.elapsedTime = task1.period;
    // task1.TickFct = &Demo_Tick;

    task2.state = start;
    task2.period = 200;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ToneSMTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for ( i = 1; i < numTasks; i++ ) {
        GCD = findGCD(GCD,tasks[i]->period);
    }

    PWM_on();
    TimerSet(GCD);
    TimerOn();

    while(1){
        for (i = 0; i < numTasks; i++){
            if (tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }
        while(!TimerFlag);
        TimerFlag = 0;

    }
    return 0;
}
