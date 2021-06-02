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