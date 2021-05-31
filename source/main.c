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

unsigned char buttonFlag = 0x00;
unsigned char rowFlag = 0xFF;

// LED Matrix SM: Displays running lights
enum Demo_States {shift};
int Demo_Tick(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char row = 0xFE;  	// (s) displaying pattern. 
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
                rowFlag = (int) (rand() % 5 + 1);
				// row = (row << 1) | 0x01;
                if (rowFlag == 1) {
                    row = 0xFE;
                }
                else if (rowFlag == 2) {
                    row = 0xFD;
                }
                else if (rowFlag == 3) {
                    row = 0xFB;
                }
                else if (rowFlag == 4) {
                    row = 0xF7;
                }
                else if (rowFlag == 5) {
                    row = 0xEF;
                }
                else { // Failsafe - select none of the rows
                    row = 0xFF;
                }
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
enum TONE_States { TONE_SMStart, TONE_wait, TONE_note };

int ToneSMTick(int state) {
    unsigned char button = ~PINB & 0x1F;
    static double noteFrequency = 0x00;

    switch(state) {
        case TONE_SMStart:
            state = TONE_wait;
            break;

        case TONE_wait:
            buttonFlag = 0x00;
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                buttonFlag = button;
                state = TONE_note;
            }
            break;

        case TONE_note:
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                buttonFlag = button;
                state = TONE_note;
            }
            break;

        default:
            state = TONE_SMStart;
            break;
    } 

    switch(state) {
        case TONE_wait:
            noteFrequency = 0;
            break;
    
        case TONE_note:
            // if (button == 0x01) { // Note C - 261.63
            //     noteFrequency = 261.63;
            // }
            // else if (button == 0x02) { // Note D - 293.66
            //     noteFrequency = 293.66;
            // }
            // else if (button == 0x04) { // Note E - 329.63
            //     noteFrequency = 329.63;
            // }
            // else if (button == 0x08) { // Note F - 349.23
            //     noteFrequency = 349.23;
            // }
            // else if (button == 0x10) { // Note G - 392.00
            //     noteFrequency = 392.00;
            // }
            // else { 
            //     noteFrequency = 0;
            // }
            break;

        default:
            break;
    }
    // set_PWM(noteFrequency);

    return state;
}

enum SCORE_States { SCORE_SMStart, SCORE_wait, SCORE_compare, SCORE_waitRelease };

int ScoreSMTick(int state) {

    static unsigned char score = 0;

    switch(state) {
        case SCORE_SMStart:
            state = SCORE_wait;
            break;

        case SCORE_wait:
            set_PWM(329.63);
            if (!buttonFlag) {
                state = SCORE_wait;
            }
            else { // buttonFlag
                state = SCORE_compare;
            }
            break;

        case SCORE_compare:
            set_PWM(392.00);
            if (!buttonFlag) {
                state = SCORE_wait;
            }
            else { // buttonFlag
                state = SCORE_waitRelease;
            }
            break;

        case SCORE_waitRelease:
            // set_PWM(392.00);
            if (!buttonFlag) {
                state = SCORE_wait;
            }
            else { // buttonFlag
                state = SCORE_waitRelease;
            }
            break;

        default:
            state = SCORE_SMStart;
            break;
    } 

    switch(state) {    
        case SCORE_compare:
            if (buttonFlag == rowFlag) { 
                score++;
                // PWM_on();
                PORTA = PORTA & 0xFF;
            }
            else {
                // PWM_off();
                PORTA = PORTA & 0xFB;
            }
            break;

        default:
            // PWM_off();
            PORTA = PORTA & 0xFB;
            break;
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x04; PORTA = 0xFB;
    DDRB = 0xE0; PORTB = 0x1F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */

    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 150;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;

    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ToneSMTick;

    task3.state = start;
    task3.period = 200;
    task3.elapsedTime = task3.period;
    task3.TickFct = &ScoreSMTick;

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
