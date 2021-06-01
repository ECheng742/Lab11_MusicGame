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

unsigned char scoreFlag = 0x00;
unsigned char rowFlag = 0x00;
unsigned char lostFlag = 0x00;
unsigned char levelFlag = 0x00;
unsigned char pointsFlag = 0x00;
unsigned char deductionsFlag = 0x00;
unsigned char checkFlag = 0x00;

// LED Matrix SM: Displays running lights
enum DISPLAY_States {DISPLAY_SMStart, DISPLAY_shift};
int DisplaySMTick(int state) {
	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char row = 0xFE;  	// (s) displaying pattern. 
							            // 0: display pattern on row
							            // 1: do NOT display pattern on row
    
    static unsigned numRow = 0x00;

	// Transitions
	switch (state) {
        case DISPLAY_SMStart:
            state = DISPLAY_shift;
            break;
		case DISPLAY_shift:	
            state = DISPLAY_shift;
            break;
		default:	
            state = DISPLAY_SMStart;
			break;
	}	

	// Actions
	switch (state) {
        case DISPLAY_shift:	
            if (pattern == 0x01) { 
				pattern = 0x80;
                rowFlag = 0x00;		   
                numRow = (int) (rand() % 5 + 1);
                if (numRow == 1) {
                    row = 0xEF;
                }
                else if (numRow == 2) {
                    row = 0xF7;
                }
                else if (numRow == 3) {
                    row = 0xFB;
                }
                else if (numRow == 4) {
                    row = 0xFD;
                }
                else if (numRow == 5) {
                    row = 0xFE;
                }
                else { // Failsafe - select none of the rows
                    row = 0xFF;
                }
			} else { // Shift LED one spot to the right on current row
				pattern >>= 1;
                if (pattern == 0x01) {rowFlag = numRow;}
                else {rowFlag = 0x00;}
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
enum TONE_States { TONE_SMStart, TONE_wait, TONE_note, TONE_waitRelease };

int ToneSMTick(int state) {
    unsigned char button = ~PINB & 0x1F;
    static double noteFrequency = 0x00;
    static char hold = 0x00; // Allows player extra period of time to let go w/o loss of points

    switch(state) {
        case TONE_SMStart:
            state = TONE_wait;
            break;
        case TONE_wait:
            // PORTA = 0x04; // FIXME
            checkFlag = 0x00;
            if (!button) {
                // scoreFlag = 0x00;
                // if (rowFlag) {
                //     scoreFlag = 0x02;
                // }
                checkFlag = 0x01;
                state = TONE_wait;
            }
            else { // button
                checkFlag = 0x00;
                if (!rowFlag) {
                    // checkFlag = 0x01;
                    // deductionsFlag++; // FIXME uncomment when done testing
                    state = TONE_waitRelease;
                }
                else if (button == 0x01 && (rowFlag == 0x01)) { // Note C - 261.63
                    pointsFlag++;
                    state = TONE_note;
                }
                else if (button == 0x02 && (rowFlag == 0x02)) { // Note D - 293.66
                    pointsFlag++;
                    state = TONE_note;
                }
                else if (button == 0x04 && (rowFlag == 0x03)) { // Note E - 329.63
                    pointsFlag++;
                    state = TONE_note;
                }
                else if (button == 0x08 && (rowFlag == 0x04)) { // Note F - 349.23
                    pointsFlag++;
                    state = TONE_note;
                }
                else if (button == 0x10 && (rowFlag == 0x05)) { // Note G - 392.00
                    pointsFlag++;
                    state = TONE_note;
                }
                else { // Multiple buttons/doesn't match row
                    // deductionsFlag++; // FIXME uncomment when done testing
                    // penaltyFlag = 0x01;
                    state = TONE_waitRelease;
                }
            }
            break;
        case TONE_note:
            checkFlag = 0x00;
            // PORTA = 0x0C; // FIXME
            if (hold <= 1) {
                // scoreFlag = 0x00;
                state = TONE_note;
            }
            else if (!button && hold > 1) {
                // scoreFlag = 0x00;
                state = TONE_wait;
            }
            else { // button && hold > 1
                // scoreFlag = 0x02;
                // deductionsFlag++; // FIXME uncomment when done testing
                // penaltyFlag = 0x01;
                // checkFlag = 0x01;
                state = TONE_waitRelease;
            }
            hold = (hold + 1) % 3;
            break;
        case TONE_waitRelease:
            checkFlag = 0x00;
            // PORTA = 0x08; // FIXME
            if (!button) {
                // scoreFlag = 0x00;
                state = TONE_wait;
            }
            else { // button
                // scoreFlag = 0x00;
                // if (rowFlag) {
                //     scoreFlag = 0x02;
                // }
                checkFlag = 0x01;
                state = TONE_waitRelease;
            }
            break;
        default:
            state = TONE_SMStart;
            break;
    } 

    switch(state) {    
        case TONE_note:
            if (hold == 0x02) {
                noteFrequency = 0;
            }
            else if (button == 0x01) { // Note C - 261.63
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
                noteFrequency = 0;
            }
            break;

        default:
            noteFrequency = 0;
            break;
    }
    set_PWM(noteFrequency);

    return state;
}

enum PENALTY_States { PENALTY_SMStart, PENALTY_idle };

int PenaltySMTick(int state) {
    switch(state) {
        case PENALTY_SMStart:
            state = PENALTY_idle;
            break;
        
        case PENALTY_idle:
            if (checkFlag) {
                if (rowFlag) {
                    deductionsFlag++; // FIXME uncomment when done testing
                }
            }
            state = PENALTY_idle;
            break;

        default:
            state = PENALTY_SMStart;
            break;
    }
    return state;
}

enum LEVEL_States { LEVEL_SMStart, LEVEL_compare, LEVEL_reset };
// unsigned points = 0x00; // FIXME?
int LevelSMTick(int state) {

    switch(state) {
        case LEVEL_SMStart:
            state = LEVEL_compare;
            break;

        case LEVEL_compare:
            // if (pointsFlag == 0x01) {
            //     points++;
            //     state = LEVEL_compare;
            // }
            // else if (scoreFlag == 0x02) {
            //     deductions++;
            //     state = LEVEL_compare;
            // }
            // if (deductionsFlag >= 3) {
            //     lostFlag = 0x01;
            //     state = LEVEL_reset;
            // }
            // else { // deductions < 3
            //     if (pointsFlag == 0x03) {
            //         pointsFlag = 0x00;
            //         levelFlag = 0x01;
            //     }
            //     state = LEVEL_compare;
            // }
            break;

        case LEVEL_reset:
            pointsFlag = 0x00;
            deductionsFlag = 0x00;
            // lostFlag = 0x01;
            state = LEVEL_reset;
            break;

        default:
            state = LEVEL_SMStart;
            break;
    }
    PORTA = deductionsFlag << 2;
            // PORTA = deductions << 2; // FIXME
    return state;    
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x0C; PORTA = 0xF3;
    DDRB = 0xE0; PORTB = 0x1F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */

    static task display, tone, penalty, level;
    task *tasks[] = { &display, &tone, &penalty, &level };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    display.state = start;
    display.period = 300;
    display.elapsedTime = display.period;
    display.TickFct = &DisplaySMTick;

    tone.state = start;
    tone.period = 100;
    tone.elapsedTime = tone.period;
    tone.TickFct = &ToneSMTick;

    penalty.state = start;
    penalty.period = 300;
    penalty.elapsedTime = penalty.period;
    penalty.TickFct = &PenaltySMTick;

    level.state = start;
    level.period = 100;
    level.elapsedTime = level.period;
    level.TickFct = &LevelSMTick;

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
