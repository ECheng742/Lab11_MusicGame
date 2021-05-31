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
unsigned char rowFlag = 0;

// LED Matrix SM: Displays running lights
enum Demo_States {SMStart, shift};
int Demo_Tick(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char row = 0xFE;  	// (s) displaying pattern. 
							            // 0: display pattern on row
							            // 1: do NOT display pattern on row
    
    static unsigned numRow = 0x00;

	// Transitions
	switch (state) {
        case SMStart:
            state = shift;
            break;
		case shift:	
            state = shift;
            break;
		default:	
            state = SMStart;
			break;
	}	
	// Actions
	switch (state) {
        case shift:	
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

    switch(state) {
        case TONE_SMStart:
            state = TONE_wait;
            break;
        case TONE_wait:
            PORTA = 0x04;
            // buttonFlag = 0x00;
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                if (button == 0x01 && (rowFlag == 0x01)) { // Note C - 261.63
                    state = TONE_note;
                }
                else if (button == 0x02 && (rowFlag == 0x02)) { // Note D - 293.66
                    state = TONE_note;
                }
                else if (button == 0x04 && (rowFlag == 0x03)) { // Note E - 329.63
                    state = TONE_note;
                }
                else if (button == 0x08 && (rowFlag == 0x04)) { // Note F - 349.23
                    state = TONE_note;
                }
                else if (button == 0x10 && (rowFlag == 0x05)) { // Note G - 392.00
                    state = TONE_note;
                }
                else { // Multiple buttons or Doesn't match row
                    state = TONE_waitRelease;
                }
                // buttonFlag = button;
                // state = TONE_note;
            }
            break;
        case TONE_note:
            PORTA = 0x0C;
            if (!button) {
                state = TONE_wait;
            }
            else { // button
                // buttonFlag = button;
                state = TONE_waitRelease;
            }
            break;
        case TONE_waitRelease:
            PORTA = 0x08;
            if (button == 0x00) {
                state = TONE_wait;
            }
            else { // button
                state = TONE_waitRelease;
            }
        default:
            state = TONE_SMStart;
            break;
    } 

    switch(state) {    
        case TONE_note:
            // if (!rowFlag) {
            //     PORTA = 0x00;
            // }
            // else {
            //     PORTA = 0x04;
            // }
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
                noteFrequency = 0;
            }
            break;

        case TONE_waitRelease:
            noteFrequency = 523.25;
            break;

        case TONE_wait:
            noteFrequency = 0;
            break;

        default:
            break;
    }
    set_PWM(noteFrequency);

    return state;
}

// enum SCORE_States { SCORE_SMStart, SCORE_wait, SCORE_compare, SCORE_waitRelease };

// int ScoreSMTick(int state) {

//     static unsigned char score = 0;

//     switch(state) {
//         case SCORE_SMStart:
//             state = SCORE_wait;
//             break;

//         case SCORE_wait:
//             if (!buttonFlag) {
//                 state = SCORE_wait;
//             }
//             else { // buttonFlag
//                 state = SCORE_compare;
//             }
//             break;

//         case SCORE_compare:
//             if (!buttonFlag) {
//                 state = SCORE_wait;
//             }
//             else { // buttonFlag
//                 // state = SCORE_waitRelease;
//             }
//             break;

//         case SCORE_waitRelease:
//             // set_PWM(392.00);
//             if (!buttonFlag) {
//                 state = SCORE_wait;
//             }
//             else { // buttonFlag
//                 state = SCORE_waitRelease;
//             }
//             break;

//         default:
//             state = SCORE_SMStart;
//             break;
//     } 

//     switch(state) {    
//         case SCORE_compare:
//             if (buttonFlag == rowFlag) { 
//             set_PWM(261.63); 
//                 score++;
//                 // PWM_on();
//                 PORTA = 0x00;
//             }
//             else {
//             set_PWM(392.00);                
//                 // PWM_off();
//                 PORTA = 0x04;
//             }
//             break;

//         default:
//             set_PWM(329.63);
//             // PWM_off();
//             PORTA = 0x00;
//             break;
//     }

//     return state;
// }

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x0C; PORTA = 0xF3;
    DDRB = 0xE0; PORTB = 0x1F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */

    static task task1, task2;
    task *tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 500;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;

    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ToneSMTick;

    // task3.state = start;
    // task3.period = 100;
    // task3.elapsedTime = task3.period;
    // task3.TickFct = &ScoreSMTick;

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
