/*
   Original code by "Pascal Piazzalunga" - http://www.serveurperso.com
   Adapted for bitbox by makapuf - makapuf2@gmail.com

   Embedded version with no FAT access

   mod reference : https://www.aes.id.au/modformat.html
 */

#include <string.h>
#include "mod32.h"
#include <fatfs/ff.h>
#include <bitbox.h>
#include <stdlib.h> // rand

struct Player mod32_player;

const struct Mod *mod;

// struct soundBuffer SoundBuffer;
struct Mixer mixer;

extern FIL file;

#define min(X,Y) ( (X) < (Y) ? (X) : (Y) )


// Effects
#define ARPEGGIO              0x0
#define PORTAMENTOUP          0x1
#define PORTAMENTODOWN        0x2
#define TONEPORTAMENTO        0x3
#define VIBRATO               0x4
#define PORTAMENTOVOLUMESLIDE 0x5
#define VIBRATOVOLUMESLIDE    0x6
#define TREMOLO               0x7
#define SETCHANNELPANNING     0x8
#define SETSAMPLEOFFSET       0x9
#define VOLUMESLIDE           0xA
#define JUMPTOORDER           0xB
#define SETVOLUME             0xC
#define BREAKPATTERNTOROW     0xD
#define SETSPEED              0xF

// 0xE subset
#define SETFILTER             0x0
#define FINEPORTAMENTOUP      0x1
#define FINEPORTAMENTODOWN    0x2
#define GLISSANDOCONTROL      0x3
#define SETVIBRATOWAVEFORM    0x4
#define SETFINETUNE           0x5
#define PATTERNLOOP           0x6
#define SETTREMOLOWAVEFORM    0x7
#define RETRIGGERNOTE         0x9
#define FINEVOLUMESLIDEUP     0xA
#define FINEVOLUMESLIDEDOWN   0xB
#define NOTECUT               0xC
#define NOTEDELAY             0xD
#define PATTERNDELAY          0xE
#define INVERTLOOP            0xF


// Sorted Amiga periods
const uint16_t amigaPeriods[296] = {
    907, 900, 894, 887, 881, 875, 868, 862, //  -8 to -1
    856, 850, 844, 838, 832, 826, 820, 814, // C-1 to +7
    808, 802, 796, 791, 785, 779, 774, 768, // C#1 to +7
    762, 757, 752, 746, 741, 736, 730, 725, // D-1 to +7
    720, 715, 709, 704, 699, 694, 689, 684, // D#1 to +7
    678, 675, 670, 665, 660, 655, 651, 646, // E-1 to +7
    640, 636, 632, 628, 623, 619, 614, 610, // F-1 to +7
    604, 601, 597, 592, 588, 584, 580, 575, // F#1 to +7
    570, 567, 563, 559, 555, 551, 547, 543, // G-1 to +7
    538, 535, 532, 528, 524, 520, 516, 513, // G#1 to +7
    508, 505, 502, 498, 494, 491, 487, 484, // A-1 to +7
    480, 477, 474, 470, 467, 463, 460, 457, // A#1 to +7
    453, 450, 447, 444, 441, 437, 434, 431, // B-1 to +7
    428, 425, 422, 419, 416, 413, 410, 407, // C-2 to +7
    404, 401, 398, 395, 392, 390, 387, 384, // C#2 to +7
    381, 379, 376, 373, 370, 368, 365, 363, // D-2 to +7
    360, 357, 355, 352, 350, 347, 345, 342, // D#2 to +7
    339, 337, 335, 332, 330, 328, 325, 323, // E-2 to +7
    320, 318, 316, 314, 312, 309, 307, 305, // F-2 to +7
    302, 300, 298, 296, 294, 292, 290, 288, // F#2 to +7
    285, 284, 282, 280, 278, 276, 274, 272, // G-2 to +7
    269, 268, 266, 264, 262, 260, 258, 256, // G#2 to +7
    254, 253, 251, 249, 247, 245, 244, 242, // A-2 to +7
    240, 238, 237, 235, 233, 232, 230, 228, // A#2 to +7
    226, 225, 223, 222, 220, 219, 217, 216, // B-2 to +7
    214, 212, 211, 209, 208, 206, 205, 203, // C-3 to +7
    202, 200, 199, 198, 196, 195, 193, 192, // C#3 to +7
    190, 189, 188, 187, 185, 184, 183, 181, // D-3 to +7
    180, 179, 177, 176, 175, 174, 172, 171, // D#3 to +7
    170, 169, 167, 166, 165, 164, 163, 161, // E-3 to +7
    160, 159, 158, 157, 156, 155, 154, 152, // F-3 to +7
    151, 150, 149, 148, 147, 146, 145, 144, // F#3 to +7
    143, 142, 141, 140, 139, 138, 137, 136, // G-3 to +7
    135, 134, 133, 132, 131, 130, 129, 128, // G#3 to +7
    127, 126, 125, 125, 123, 123, 122, 121, // A-3 to +7
    120, 119, 118, 118, 117, 116, 115, 114, // A#3 to +7
    113, 113, 112, 111, 110, 109, 109, 108  // B-3 to +7
};

const uint8_t sine[64] = {
    0,  24,  49,  74,  97, 120, 141, 161,
    180, 197, 212, 224, 235, 244, 250, 253,
    255, 253, 250, 244, 235, 224, 212, 197,
    180, 161, 141, 120,  97,  74,  49,  24
};

// loads a header
void loadHeader() {
    // get number of patterns
    mod32_player.numberOfPatterns = 0;
    for (int i = 0; i < 128; i++) {
        if (mod->order[i] > mod32_player.numberOfPatterns)
            mod32_player.numberOfPatterns = mod->order[i];
    }
    mod32_player.numberOfPatterns++;

    // get nb of channels : CHNx or CHyy or else 4
    if (mod->tag[1] == 'C' && mod->tag[2]=='H') {
        if (mod->tag[3]=='N') {
            mod32_player.numberOfChannels = mod->tag[0] - '0';
        } else {
            mod32_player.numberOfChannels = (mod->tag[0] - '0') * 10 + mod->tag[1] - '0';
        }
    } else {
        mod32_player.numberOfChannels = 4;
    }
    
    // check if enough channels to be played
    if (mod32_player.numberOfChannels > CHANNELS) {
        mod32_player.numberOfChannels=0; // stop playing
        message("Not enough channels to play this song ! Will respectfully crash here.\n");
        die (1,1);
    }

    message("loading %s\n",mod->name);
    message(" * %d patterns %d channels\n",mod32_player.numberOfPatterns, mod32_player.numberOfChannels );
}

// conversions from word to len (swap bytes and *2)
uint16_t h2len(uint16_t n) {return (n&0xff)<<9 | (n>>8)*2;}

// loads the sample definitions
void loadSamples() {
    uint32_t fileOffset = sizeof(struct Mod) + mod32_player.numberOfPatterns * ROWS * mod32_player.numberOfChannels * 4 -1;

    for (int i=0; i<SAMPLES; i++) {
        
        const struct Sample *smp = &mod->samples[i];
        const uint16_t len= h2len(smp->length);
        if (len) {
            message("sample %d '%022s'",i,smp->name);
            message(" begin @%d, len %5d, volume %2d ",fileOffset, len, smp->volume);

            mixer.sampleBegin[i] = fileOffset;
            mixer.sampleEnd[i] = fileOffset + len;
            if (h2len(smp->loopLength) > 2) {
                message(" (loop start:%d len:%d)",h2len(smp->loopBegin),h2len(smp->loopLength));
                mixer.sampleloopBegin[i] = fileOffset + h2len(smp->loopBegin);
                mixer.sampleLoopLength[i] = h2len(smp->loopLength);
                mixer.sampleLoopEnd[i] = mixer.sampleloopBegin[i] + mixer.sampleLoopLength[i];
            } else {
                mixer.sampleloopBegin[i] = 0;
                mixer.sampleLoopLength[i] = 0;
                mixer.sampleLoopEnd[i] = 0;
            }
            fileOffset += len;
            message("\n");
        }
    }
}

// loads a pattern
void loadPattern(uint8_t pattern) {

    message("loading pattern %d\n",pattern);
    message("loading at pos %d + %d\n", sizeof(struct Mod),pattern * ROWS  * mod32_player.numberOfChannels * 4 );

    uint8_t *temp =  (uint8_t *) mod + \
        sizeof(struct Mod) + pattern * ROWS * mod32_player.numberOfChannels * 4;

    for (int row = 0; row < ROWS; row++) {
        for (int channel = 0; channel < mod32_player.numberOfChannels; channel++) {
            mod32_player.currentPattern.sampleNumber[row][channel] = (temp[0] & 0xF0) + (temp[2] >> 4);

            uint16_t amigaPeriod = ( (temp[0] & 0xF) << 8 ) + temp[1];
            mod32_player.currentPattern.note[row][channel] = NONOTE;
            // wat is this ? why not keep amigaperiods ? 
            for (int i = 1; i < 37; i++)
                if (amigaPeriod > amigaPeriods[i * 8] - 3 &&
                    amigaPeriod < amigaPeriods[i * 8] + 3) {
                    mod32_player.currentPattern.note[row][channel] = i * 8;
                    break;
                }

            #if 0 // display pattern
                int note = mod32_player.currentPattern.note[row][channel];
                int sample = mod32_player.currentPattern.sampleNumber[row][channel];
                if (note==NONOTE)
                    message ("--    ");
                else
                    message("%2x[%x] ", note, sample );
            #endif 

            mod32_player.currentPattern.effectNumber[row][channel] = temp[2] & 0xF;
            mod32_player.currentPattern.effectParameter[row][channel] = temp[3];

            temp += 4; 
        }
        #if 0
        message("\n");
        #endif 
    }
    message("end pattern\n");
}
void portamento(uint8_t channel) {
    if (mod32_player.lastAmigaPeriod[channel] < mod32_player.portamentoNote[channel]) {
        mod32_player.lastAmigaPeriod[channel] += mod32_player.portamentoSpeed[channel];
        if (mod32_player.lastAmigaPeriod[channel] > mod32_player.portamentoNote[channel])
            mod32_player.lastAmigaPeriod[channel] = mod32_player.portamentoNote[channel];
    }
    if (mod32_player.lastAmigaPeriod[channel] > mod32_player.portamentoNote[channel]) {
        mod32_player.lastAmigaPeriod[channel] -= mod32_player.portamentoSpeed[channel];
        if (mod32_player.lastAmigaPeriod[channel] < mod32_player.portamentoNote[channel])
            mod32_player.lastAmigaPeriod[channel] = mod32_player.portamentoNote[channel];
    }
    mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];
}

void vibrato(uint8_t channel) {
    uint16_t delta;
    uint16_t temp;

    temp = mod32_player.vibratoPos[channel] & 31;

    switch(mod32_player.waveControl[channel] & 3) {
        case 0:
            delta = sine[temp];
            break;
        case 1:
            temp <<= 3;
            if (mod32_player.vibratoPos[channel] < 0)
                temp = 255 - temp;
            delta = temp;
            break;
        case 2:
            delta = 255;
            break;
        case 3: 
        default: 
            delta = rand() & 255;
            break;
    }

    delta *= mod32_player.vibratoDepth[channel];
    delta >>= 7;

    if (mod32_player.vibratoPos[channel] >= 0)
        mixer.channelFrequency[channel] = mod32_player.amiga / (mod32_player.lastAmigaPeriod[channel] + delta);
    else
        mixer.channelFrequency[channel] = mod32_player.amiga / (mod32_player.lastAmigaPeriod[channel] - delta);

    mod32_player.vibratoPos[channel] += mod32_player.vibratoSpeed[channel];
    if (mod32_player.vibratoPos[channel] > 31) mod32_player.vibratoPos[channel] -= 64;
}

void tremolo(uint8_t channel) {
    uint16_t delta;
    uint16_t temp;

    temp = mod32_player.tremoloPos[channel] & 31;

    switch(mod32_player.waveControl[channel] & 3) {
        case 0:
            delta = sine[temp];
            break;
        case 1:
            temp <<= 3;
            if (mod32_player.tremoloPos[channel] < 0)
                temp = 255 - temp;
            delta = temp;
            break;
        case 2:
            delta = 255;
            break;
        case 3:
        default: 
            delta = rand() & 255;
            break;
    }

    delta *= mod32_player.tremoloDepth[channel];
    delta >>= 6;

    if (mod32_player.tremoloPos[channel] >= 0) {
        if (mod32_player.volume[channel] + delta > 64) delta = 64 - mod32_player.volume[channel];
        mixer.channelVolume[channel] = mod32_player.volume[channel] + delta;
    } else {
        if (mod32_player.volume[channel] - delta < 0) delta = mod32_player.volume[channel];
        mixer.channelVolume[channel] = mod32_player.volume[channel] - delta;
    }

    mod32_player.tremoloPos[channel] += mod32_player.tremoloSpeed[channel];
    if (mod32_player.tremoloPos[channel] > 31) mod32_player.tremoloPos[channel] -= 64;
}

void processRow() 
{
    mod32_player.lastRow = mod32_player.row++;
    bool jumpFlag = false;
    bool breakFlag = false;
    for (int channel = 0; channel < mod32_player.numberOfChannels; channel++) {

        uint8_t sampleNumber = mod32_player.currentPattern.sampleNumber[mod32_player.lastRow][channel];
        uint16_t note = mod32_player.currentPattern.note[mod32_player.lastRow][channel];

        uint8_t effectNumber = mod32_player.currentPattern.effectNumber[mod32_player.lastRow][channel];
        uint8_t effectParameter = mod32_player.currentPattern.effectParameter[mod32_player.lastRow][channel];
        uint8_t effectParameterX = effectParameter >> 4;
        uint8_t effectParameterY = effectParameter & 0xF;

        uint16_t sampleOffset = 0;

        if (sampleNumber) {
            mod32_player.lastSampleNumber[channel] = sampleNumber - 1; 
            if ( !(effectParameter == 0xE && effectParameterX == NOTEDELAY) )
                mod32_player.volume[channel] = mod->samples[mod32_player.lastSampleNumber[channel]].volume;
        }

        if (note != NONOTE) {
            mod32_player.lastNote[channel] = note;
            mod32_player.amigaPeriod[channel] = amigaPeriods[note + mod->samples[mod32_player.lastSampleNumber[channel]].fineTune];
            // DEBUG
            // message("channel:%d note : %x sample %d finetune : %d note period: %d\n",channel, note, mod32_player.lastSampleNumber[channel], mod->samples[mod32_player.lastSampleNumber[channel]].fineTune,amigaPeriods[note] );

            if (effectNumber != TONEPORTAMENTO && effectNumber != PORTAMENTOVOLUMESLIDE)
                mod32_player.lastAmigaPeriod[channel] = mod32_player.amigaPeriod[channel];

            if ( !(mod32_player.waveControl[channel] & 0x80) ) mod32_player.vibratoPos[channel] = 0;
            if ( !(mod32_player.waveControl[channel] & 0x08) ) mod32_player.tremoloPos[channel] = 0;
        }
        switch(effectNumber) {
            case TONEPORTAMENTO:
                if (effectParameter) mod32_player.portamentoSpeed[channel] = effectParameter;
                mod32_player.portamentoNote[channel] = mod32_player.amigaPeriod[channel];
                note = NONOTE;
                break;

            case VIBRATO:
                if (effectParameterX) mod32_player.vibratoSpeed[channel] = effectParameterX;
                if (effectParameterY) mod32_player.vibratoDepth[channel] = effectParameterY;
                break;

            case PORTAMENTOVOLUMESLIDE:
                mod32_player.portamentoNote[channel] = mod32_player.amigaPeriod[channel];
                note = NONOTE;
                break;

            case TREMOLO:
                if (effectParameterX) mod32_player.tremoloSpeed[channel] = effectParameterX;
                if (effectParameterY) mod32_player.tremoloDepth[channel] = effectParameterY;
                break;

            case SETCHANNELPANNING:
                mixer.channelPanning[channel] = effectParameter >> 1;
                break;

            case SETSAMPLEOFFSET:
                sampleOffset = effectParameter << 8;
                int samplelen = h2len(mod->samples[mod32_player.lastSampleNumber[channel]].length);
                if (sampleOffset > samplelen) 
                    sampleOffset = samplelen;
                break;

            case JUMPTOORDER:
                mod32_player.orderIndex = effectParameter;
                if (mod32_player.orderIndex >= mod->songLength)
                    mod32_player.orderIndex = 0;
                mod32_player.row = 0;
                jumpFlag = true;
                break;

            case SETVOLUME:
                if (effectParameter > 64) mod32_player.volume[channel] = 64;
                else mod32_player.volume[channel] = effectParameter;
                break;

            case BREAKPATTERNTOROW:
                mod32_player.row = effectParameterX * 10 + effectParameterY;
                if (mod32_player.row >= ROWS)
                    mod32_player.row = 0;
                if (!jumpFlag && !breakFlag) {
                    mod32_player.orderIndex++;
                    if (mod32_player.orderIndex >= mod->songLength)
                        mod32_player.orderIndex = 0;
                }
                breakFlag = true;
                break;

            case 0xE:
                switch(effectParameterX) {
                    case FINEPORTAMENTOUP:
                        mod32_player.lastAmigaPeriod[channel] -= effectParameterY;
                        break;

                    case FINEPORTAMENTODOWN:
                        mod32_player.lastAmigaPeriod[channel] += effectParameterY;
                        break;

                    case SETVIBRATOWAVEFORM:
                        mod32_player.waveControl[channel] &= 0xF0;
                        mod32_player.waveControl[channel] |= effectParameterY;
                        break;
/* // Disabled finetune so that mod can be a const
                    case SETFINETUNE:
                        mod->samples[mod32_player.lastSampleNumber[channel]].fineTune = effectParameterY;
                        if (mod->samples[mod32_player.lastSampleNumber[channel]].fineTune > 7)
                            mod->samples[mod32_player.lastSampleNumber[channel]].fineTune -= 16;
                        break;
*/
                    case PATTERNLOOP:
                        if (effectParameterY) {
                            if (mod32_player.patternLoopCount[channel])
                                mod32_player.patternLoopCount[channel]--;
                            else
                                mod32_player.patternLoopCount[channel] = effectParameterY;
                            if (mod32_player.patternLoopCount[channel])
                                mod32_player.row = mod32_player.patternLoopRow[channel] - 1;
                        } else
                            mod32_player.patternLoopRow[channel] = mod32_player.row;
                        break;

                    case SETTREMOLOWAVEFORM:
                        mod32_player.waveControl[channel] &= 0xF;
                        mod32_player.waveControl[channel] |= effectParameterY << 4;
                        break;

                    case FINEVOLUMESLIDEUP:
                        mod32_player.volume[channel] += effectParameterY;
                        if (mod32_player.volume[channel] > 64) mod32_player.volume[channel] = 64;
                        break;

                    case FINEVOLUMESLIDEDOWN:
                        mod32_player.volume[channel] -= effectParameterY;
                        if (mod32_player.volume[channel] < 0) mod32_player.volume[channel] = 0;
                        break;

                    case NOTECUT:
                        note = NONOTE;
                        break;

                    case PATTERNDELAY:
                        mod32_player.patternDelay = effectParameterY;
                        break;

                    case INVERTLOOP:

                        break;
                }
                break;

            case SETSPEED:
                if (effectParameter < 0x20)
                    mod32_player.speed = effectParameter;
                else
                    mod32_player.samplesPerTick = BITBOX_SAMPLERATE / (2 * effectParameter / 5);
                break;
        }

        // must recalculate channelfrequency ?
        if ( note != NONOTE || ( 
            mod32_player.lastAmigaPeriod[channel] &&
            effectNumber != VIBRATO && 
            effectNumber != VIBRATOVOLUMESLIDE &&
            !(effectNumber == 0xE && effectParameterX == NOTEDELAY) ) 
            )
            mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];

        if (note != NONOTE)
            mixer.channelSampleOffset[channel] = sampleOffset << DIVIDER;

        if (sampleNumber)
            mixer.channelSampleNumber[channel] = mod32_player.lastSampleNumber[channel];

        if (effectNumber != TREMOLO)
            mixer.channelVolume[channel] = mod32_player.volume[channel];

    }
}


void processTick() {
    uint16_t tempNote;

    for (uint8_t channel = 0; channel < mod32_player.numberOfChannels; channel++) {

        if (mod32_player.lastAmigaPeriod[channel]) {

            uint8_t sampleNumber = mod32_player.currentPattern.sampleNumber[mod32_player.lastRow][channel];
            uint16_t note = mod32_player.currentPattern.note[mod32_player.lastRow][channel];
            uint8_t effectNumber = mod32_player.currentPattern.effectNumber[mod32_player.lastRow][channel];
            uint8_t effectParameter = mod32_player.currentPattern.effectParameter[mod32_player.lastRow][channel];
            uint8_t effectParameterX = effectParameter >> 4;
            uint8_t effectParameterY = effectParameter & 0xF;
            
            switch(effectNumber) {
                case ARPEGGIO:
                    if (effectParameter)
                        switch(mod32_player.tick % 3) {
                            case 0:
                                mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];
                                break;
                            case 1:
                                tempNote = mod32_player.lastNote[channel] + effectParameterX * 8 + mod->samples[mod32_player.lastSampleNumber[channel]].fineTune;
                                if (tempNote < 296) mixer.channelFrequency[channel] = mod32_player.amiga / amigaPeriods[tempNote];
                                break;
                            case 2:
                                tempNote = mod32_player.lastNote[channel] + effectParameterY * 8 + mod->samples[mod32_player.lastSampleNumber[channel]].fineTune;
                                if (tempNote < 296) mixer.channelFrequency[channel] = mod32_player.amiga / amigaPeriods[tempNote];
                                break;
                        }
                    break;

                case PORTAMENTOUP:
                    mod32_player.lastAmigaPeriod[channel] -= effectParameter;
                    if (mod32_player.lastAmigaPeriod[channel] < 113) mod32_player.lastAmigaPeriod[channel] = 113;
                    mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];
                    break;

                case PORTAMENTODOWN:
                    mod32_player.lastAmigaPeriod[channel] += effectParameter;
                    if (mod32_player.lastAmigaPeriod[channel] > 856) mod32_player.lastAmigaPeriod[channel] = 856;
                    mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];
                    break;

                case TONEPORTAMENTO:
                    portamento(channel);
                    break;

                case VIBRATO:
                    vibrato(channel);
                    break;

                case PORTAMENTOVOLUMESLIDE:
                    portamento(channel);
                    mod32_player.volume[channel] += effectParameterX - effectParameterY;
                    if (mod32_player.volume[channel] < 0) mod32_player.volume[channel] = 0;
                    else if (mod32_player.volume[channel] > 64) mod32_player.volume[channel] = 64;
                    mixer.channelVolume[channel] = mod32_player.volume[channel];
                    break;

                case VIBRATOVOLUMESLIDE:
                    vibrato(channel);
                    mod32_player.volume[channel] += effectParameterX - effectParameterY;
                    if (mod32_player.volume[channel] < 0) mod32_player.volume[channel] = 0;
                    else if (mod32_player.volume[channel] > 64) mod32_player.volume[channel] = 64;
                    mixer.channelVolume[channel] = mod32_player.volume[channel];
                    break;

                case TREMOLO:
                    tremolo(channel);
                    break;

                case VOLUMESLIDE:
                    mod32_player.volume[channel] += effectParameterX - effectParameterY;
                    if (mod32_player.volume[channel] < 0) mod32_player.volume[channel] = 0;
                    else if (mod32_player.volume[channel] > 64) mod32_player.volume[channel] = 64;
                    mixer.channelVolume[channel] = mod32_player.volume[channel];
                    break;

                case 0xE:
                    switch(effectParameterX) {
                        case RETRIGGERNOTE:
                            if (!effectParameterY) break;
                            if ( !(mod32_player.tick % effectParameterY) ) {
                                mixer.channelSampleOffset[channel] = 0;
                            }
                            break;

                        case NOTECUT:
                            if (mod32_player.tick == effectParameterY)
                                mixer.channelVolume[channel] = mod32_player.volume[channel] = 0;
                            break;

                        case NOTEDELAY:
                            if (mod32_player.tick == effectParameterY) {
                                if (sampleNumber) mod32_player.volume[channel] = mod->samples[mod32_player.lastSampleNumber[channel]].volume;
                                if (note != NONOTE) mixer.channelSampleOffset[channel] = 0;
                                mixer.channelFrequency[channel] = mod32_player.amiga / mod32_player.lastAmigaPeriod[channel];
                                mixer.channelVolume[channel] = mod32_player.volume[channel];
                            }
                            break;
                    }
                    break;
            }

        }
    }
}


/* updates the mod32_player state : tick, rows or patterns */
void update_mod32_player() 
{
    if (mod32_player.tick == mod32_player.speed) {
        mod32_player.tick = 0;

        if (mod32_player.row == ROWS) {
            mod32_player.orderIndex++;
            if (mod32_player.orderIndex == mod->songLength)
                mod32_player.orderIndex = 0;
            mod32_player.row = 0;
        }

        if (mod32_player.patternDelay) {
            mod32_player.patternDelay--;
        } else {
            if (mod32_player.orderIndex != mod32_player.oldOrderIndex)
                loadPattern(mod->order[mod32_player.orderIndex]);
            mod32_player.oldOrderIndex = mod32_player.orderIndex;
            processRow();
        }

    } else {
        processTick();
    }
    mod32_player.tick++;
}

/* generates a sample from the current mod32_player status */
uint16_t gen_sample(uint16_t *buffer) 
{
    int16_t sumL = 0;
    #if MOD_STEREOSEPARATION!=64
    int16_t sumR = 0;
    #endif

    for (int channel = 0; channel < mod32_player.numberOfChannels; channel++) {
        // if (channel != 3) continue; // FIXME SOLO TRACK

        const int sample_id = mixer.channelSampleNumber[channel]; // shortcut

        // channel with no sample or empty sample : skip
        if (!mixer.channelFrequency[channel] || !mod->samples[sample_id].length) 
            continue;

        mixer.channelSampleOffset[channel] += mixer.channelFrequency[channel];

        // muted channel 
        if (!mixer.channelVolume[channel]) 
            continue;

        // determine sample pointer from sampleBegin and sample offset 
        uint32_t samplePointer = mixer.sampleBegin[sample_id] +
                        (mixer.channelSampleOffset[channel] >> DIVIDER);

        // DEBUG
        // message("sample %d @%d[%d]=%d\n",sample_id, mixer.sampleBegin[sample_id],mixer.channelSampleOffset[channel] >> DIVIDER, samplePointer);

        // loop / end of sample ?
        if (mixer.sampleLoopLength[sample_id]) {
            if (samplePointer >= mixer.sampleLoopEnd[sample_id]) {
                mixer.channelSampleOffset[channel] -= mixer.sampleLoopLength[sample_id] << DIVIDER;
                samplePointer -= mixer.sampleLoopLength[sample_id];
            }
        } else {
            if (samplePointer >= mixer.sampleEnd[sample_id]) {
                mixer.channelFrequency[channel] = 0;
                samplePointer = mixer.sampleEnd[sample_id];
            }            
        }

        // direct read from memory
        const int8_t *file = (int8_t *)mod;
        int8_t current = file[samplePointer];

        int16_t out = current;

        // Integer linear interpolation
        #ifdef MOD_ENABLE_LINEAR_INTERPOLATION
        int8_t next    = file[samplePointer+1];
        out += (next - current) * (mixer.channelSampleOffset[channel] & ((1 << DIVIDER) - 1)) >> DIVIDER;
        #endif 

        // Upscale to BITDEPTH
        out <<= BITBOX_SAMPLE_BITDEPTH - 8;

        // Channel volume
        out = out * mixer.channelVolume[channel] >> 6;

        // Channel panning
        #if MOD_STEREOSEPARATION!=64
        sumL += out * min(128 - mixer.channelPanning[channel], 64) >> 6;
        sumR += out * min(mixer.channelPanning[channel], 64) >> 6;
        #else // mono
        sumL += out;
        #endif 
    }

    // Downscale to BITDEPTH
    uint16_t sample; // left<< 8 | right

    #if MOD_STEREOSEPARATION!=64
    sumL /= mod32_player.numberOfChannels;
    sumR /= mod32_player.numberOfChannels;
    sample = ( sumL + (1 << (BITBOX_SAMPLE_BITDEPTH-1)) )<<8 | ( ( sumR+ (1 << (BITBOX_SAMPLE_BITDEPTH-1)) ) );
    #else // mono take left channel only
    sample = ((sumL / mod32_player.numberOfChannels)+128) * 0x0101;
    #endif 

    return sample;
}


void mod32_player_reset(void)
{
    mod32_player.amiga = AMIGA;
    mod32_player.samplesPerTick = BITBOX_SAMPLERATE / (2 * 125 / 5); // Hz = 2 * BPM / 5
    mod32_player.speed = 6;
    mod32_player.tick = mod32_player.speed;
    mod32_player.row = 0;

    mod32_player.orderIndex = 0;
    mod32_player.oldOrderIndex = 0xFF;
    mod32_player.patternDelay = 0;


    // reset mod32_player
    for (uint8_t channel = 0; channel < mod32_player.numberOfChannels; channel++) {
        mod32_player.patternLoopCount[channel] = 0;
        mod32_player.patternLoopRow[channel] = 0;

        mod32_player.lastAmigaPeriod[channel] = 0;

        mod32_player.waveControl[channel] = 0;

        mod32_player.vibratoSpeed[channel] = 0;
        mod32_player.vibratoDepth[channel] = 0;
        mod32_player.vibratoPos[channel] = 0;

        mod32_player.tremoloSpeed[channel] = 0;
        mod32_player.tremoloDepth[channel] = 0;
        mod32_player.tremoloPos[channel] = 0;

        mixer.channelSampleOffset[channel] = 0;
        mixer.channelFrequency[channel] = 0;
        mixer.channelVolume[channel] = 0;
        switch(channel % 4) {
            case 0:
            case 3:
                mixer.channelPanning[channel] = MOD_STEREOSEPARATION;
                break;
            default:
                mixer.channelPanning[channel] = 128 - MOD_STEREOSEPARATION;
        }
    }
}

void load_mod(const void* modfile) 
{
    if (modfile) {
        mod = (const struct Mod*) modfile; // global
        loadHeader();
        loadSamples();
    }

    mod32_player_reset();
}


void game_snd_buffer (uint16_t *stream, int size)
{
    static int sample_in_tick;
    if (!mod32_player.numberOfChannels)
        return;

    for (int i=0;i<size; i++) {
        // song status / change updating        
        if (sample_in_tick++==mod32_player.samplesPerTick) /// ROW DELETE ME !
        {
            update_mod32_player();
            sample_in_tick=0;
        }
        
        stream[i] = gen_sample(stream); 
    }
} 