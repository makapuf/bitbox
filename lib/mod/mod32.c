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

struct Player player;

struct Mod *mod;

// struct soundBuffer SoundBuffer;
struct Mixer mixer;

extern FIL file;

#define min(X,Y) ( (X) < (Y) ? (X) : (Y) )


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
    player.numberOfPatterns = 0;
    for (int i = 0; i < 128; i++) {
        if (mod->order[i] > player.numberOfPatterns)
            player.numberOfPatterns = mod->order[i];
    }
    player.numberOfPatterns++;

    // get nb of channels : CHNx or CHyy or else 4
    if (mod->tag[1] == 'C' && mod->tag[2]=='H') {
        if (mod->tag[3]=='N') {
            player.numberOfChannels = mod->tag[0] - '0';
        } else {
            player.numberOfChannels = (mod->tag[0] - '0') * 10 + mod->tag[1] - '0';
        }
    } else {
        player.numberOfChannels = 4;
    }
    
    // check if enough channels to be played
    if (player.numberOfChannels > CHANNELS) {
        player.numberOfChannels=0; // stop playing
        message("Not enough channels to play this song ! Will respectfully crash here.\n");
        die (1,1);
    }

    message("loading %s\n",mod->name);
    message(" * %d patterns %d channels\n",player.numberOfPatterns, player.numberOfChannels );
}

// conversions from word to len (swap bytes and *2)
uint16_t h2len(uint16_t n) {return (n&0xff)<<9 | (n>>8)*2;}

// loads the sample definitions
void loadSamples() {
    uint32_t fileOffset = sizeof(struct Mod) + player.numberOfPatterns * ROWS * player.numberOfChannels * 4 -1;

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
    message("loading at pos %d + %d\n", sizeof(struct Mod),pattern * ROWS  * player.numberOfChannels * 4 );

    uint8_t *temp =  (uint8_t *) mod + \
        sizeof(struct Mod) + pattern * ROWS * player.numberOfChannels * 4;

    for (int row = 0; row < ROWS; row++) {
        for (int channel = 0; channel < player.numberOfChannels; channel++) {
            player.currentPattern.sampleNumber[row][channel] = (temp[0] & 0xF0) + (temp[2] >> 4);

            uint16_t amigaPeriod = ( (temp[0] & 0xF) << 8 ) + temp[1];
            player.currentPattern.note[row][channel] = NONOTE;
            // wat is this ? why not keep amigaperiods ? 
            for (int i = 1; i < 37; i++)
                if (amigaPeriod > amigaPeriods[i * 8] - 3 &&
                    amigaPeriod < amigaPeriods[i * 8] + 3) {
                    player.currentPattern.note[row][channel] = i * 8;
                    break;
                }

            #if 0 // display pattern
                int note = player.currentPattern.note[row][channel];
                int sample = player.currentPattern.sampleNumber[row][channel];
                if (note==NONOTE)
                    message ("--    ");
                else
                    message("%2x[%x] ", note, sample );
            #endif 

            player.currentPattern.effectNumber[row][channel] = temp[2] & 0xF;
            player.currentPattern.effectParameter[row][channel] = temp[3];

            temp += 4; 
        }
        // message("\n");
    }
    message("end pattern\n");
}
void portamento(uint8_t channel) {
    if (player.lastAmigaPeriod[channel] < player.portamentoNote[channel]) {
        player.lastAmigaPeriod[channel] += player.portamentoSpeed[channel];
        if (player.lastAmigaPeriod[channel] > player.portamentoNote[channel])
            player.lastAmigaPeriod[channel] = player.portamentoNote[channel];
    }
    if (player.lastAmigaPeriod[channel] > player.portamentoNote[channel]) {
        player.lastAmigaPeriod[channel] -= player.portamentoSpeed[channel];
        if (player.lastAmigaPeriod[channel] < player.portamentoNote[channel])
            player.lastAmigaPeriod[channel] = player.portamentoNote[channel];
    }
    mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];
}

void vibrato(uint8_t channel) {
    uint16_t delta;
    uint16_t temp;

    temp = player.vibratoPos[channel] & 31;

    switch(player.waveControl[channel] & 3) {
        case 0:
            delta = sine[temp];
            break;
        case 1:
            temp <<= 3;
            if (player.vibratoPos[channel] < 0)
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

    delta *= player.vibratoDepth[channel];
    delta >>= 7;

    if (player.vibratoPos[channel] >= 0)
        mixer.channelFrequency[channel] = player.amiga / (player.lastAmigaPeriod[channel] + delta);
    else
        mixer.channelFrequency[channel] = player.amiga / (player.lastAmigaPeriod[channel] - delta);

    player.vibratoPos[channel] += player.vibratoSpeed[channel];
    if (player.vibratoPos[channel] > 31) player.vibratoPos[channel] -= 64;
}

void tremolo(uint8_t channel) {
    uint16_t delta;
    uint16_t temp;

    temp = player.tremoloPos[channel] & 31;

    switch(player.waveControl[channel] & 3) {
        case 0:
            delta = sine[temp];
            break;
        case 1:
            temp <<= 3;
            if (player.tremoloPos[channel] < 0)
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

    delta *= player.tremoloDepth[channel];
    delta >>= 6;

    if (player.tremoloPos[channel] >= 0) {
        if (player.volume[channel] + delta > 64) delta = 64 - player.volume[channel];
        mixer.channelVolume[channel] = player.volume[channel] + delta;
    } else {
        if (player.volume[channel] - delta < 0) delta = player.volume[channel];
        mixer.channelVolume[channel] = player.volume[channel] - delta;
    }

    player.tremoloPos[channel] += player.tremoloSpeed[channel];
    if (player.tremoloPos[channel] > 31) player.tremoloPos[channel] -= 64;
}

void processRow() 
{
    player.lastRow = player.row++;
    bool jumpFlag = false;
    bool breakFlag = false;
    for (int channel = 0; channel < player.numberOfChannels; channel++) {

        uint8_t sampleNumber = player.currentPattern.sampleNumber[player.lastRow][channel];
        uint16_t note = player.currentPattern.note[player.lastRow][channel];

        uint8_t effectNumber = player.currentPattern.effectNumber[player.lastRow][channel];
        uint8_t effectParameter = player.currentPattern.effectParameter[player.lastRow][channel];
        uint8_t effectParameterX = effectParameter >> 4;
        uint8_t effectParameterY = effectParameter & 0xF;

        uint16_t sampleOffset = 0;

        if (sampleNumber) {
            player.lastSampleNumber[channel] = sampleNumber - 1; // FIXME FIXME
            if ( !(effectParameter == 0xE && effectParameterX == NOTEDELAY) )
                player.volume[channel] = mod->samples[player.lastSampleNumber[channel]].volume;
        }

        if (note != NONOTE) {
            player.lastNote[channel] = note;
            player.amigaPeriod[channel] = amigaPeriods[note + mod->samples[player.lastSampleNumber[channel]].fineTune];
            // DEBUG
            // message("channel:%d note : %x sample %d finetune : %d note period: %d\n",channel, note, player.lastSampleNumber[channel], mod->samples[player.lastSampleNumber[channel]].fineTune,amigaPeriods[note] );

            if (effectNumber != TONEPORTAMENTO && effectNumber != PORTAMENTOVOLUMESLIDE)
                player.lastAmigaPeriod[channel] = player.amigaPeriod[channel];

            if ( !(player.waveControl[channel] & 0x80) ) player.vibratoPos[channel] = 0;
            if ( !(player.waveControl[channel] & 0x08) ) player.tremoloPos[channel] = 0;
        }
        switch(effectNumber) {
            case TONEPORTAMENTO:
                if (effectParameter) player.portamentoSpeed[channel] = effectParameter;
                player.portamentoNote[channel] = player.amigaPeriod[channel];
                note = NONOTE;
                break;

            case VIBRATO:
                if (effectParameterX) player.vibratoSpeed[channel] = effectParameterX;
                if (effectParameterY) player.vibratoDepth[channel] = effectParameterY;
                break;

            case PORTAMENTOVOLUMESLIDE:
                player.portamentoNote[channel] = player.amigaPeriod[channel];
                note = NONOTE;
                break;

            case TREMOLO:
                if (effectParameterX) player.tremoloSpeed[channel] = effectParameterX;
                if (effectParameterY) player.tremoloDepth[channel] = effectParameterY;
                break;

            case SETCHANNELPANNING:
                mixer.channelPanning[channel] = effectParameter >> 1;
                break;

            case SETSAMPLEOFFSET:
                sampleOffset = effectParameter << 8;
                int samplelen = h2len(mod->samples[player.lastSampleNumber[channel]].length);
                if (sampleOffset > samplelen) 
                    sampleOffset = samplelen;
                break;

            case JUMPTOORDER:
                player.orderIndex = effectParameter;
                if (player.orderIndex >= mod->songLength)
                    player.orderIndex = 0;
                player.row = 0;
                jumpFlag = true;
                break;

            case SETVOLUME:
                if (effectParameter > 64) player.volume[channel] = 64;
                else player.volume[channel] = effectParameter;
                break;

            case BREAKPATTERNTOROW:
                player.row = effectParameterX * 10 + effectParameterY;
                if (player.row >= ROWS)
                    player.row = 0;
                if (!jumpFlag && !breakFlag) {
                    player.orderIndex++;
                    if (player.orderIndex >= mod->songLength)
                        player.orderIndex = 0;
                }
                breakFlag = true;
                break;

            case 0xE:
                switch(effectParameterX) {
                    case FINEPORTAMENTOUP:
                        player.lastAmigaPeriod[channel] -= effectParameterY;
                        break;

                    case FINEPORTAMENTODOWN:
                        player.lastAmigaPeriod[channel] += effectParameterY;
                        break;

                    case SETVIBRATOWAVEFORM:
                        player.waveControl[channel] &= 0xF0;
                        player.waveControl[channel] |= effectParameterY;
                        break;

                    case SETFINETUNE:
                        mod->samples[player.lastSampleNumber[channel]].fineTune = effectParameterY;
                        if (mod->samples[player.lastSampleNumber[channel]].fineTune > 7)
                            mod->samples[player.lastSampleNumber[channel]].fineTune -= 16;
                        break;

                    case PATTERNLOOP:
                        if (effectParameterY) {
                            if (player.patternLoopCount[channel])
                                player.patternLoopCount[channel]--;
                            else
                                player.patternLoopCount[channel] = effectParameterY;
                            if (player.patternLoopCount[channel])
                                player.row = player.patternLoopRow[channel] - 1;
                        } else
                            player.patternLoopRow[channel] = player.row;
                        break;

                    case SETTREMOLOWAVEFORM:
                        player.waveControl[channel] &= 0xF;
                        player.waveControl[channel] |= effectParameterY << 4;
                        break;

                    case FINEVOLUMESLIDEUP:
                        player.volume[channel] += effectParameterY;
                        if (player.volume[channel] > 64) player.volume[channel] = 64;
                        break;

                    case FINEVOLUMESLIDEDOWN:
                        player.volume[channel] -= effectParameterY;
                        if (player.volume[channel] < 0) player.volume[channel] = 0;
                        break;

                    case NOTECUT:
                        note = NONOTE;
                        break;

                    case PATTERNDELAY:
                        player.patternDelay = effectParameterY;
                        break;

                    case INVERTLOOP:

                        break;
                }
                break;

            case SETSPEED:
                if (effectParameter < 0x20)
                    player.speed = effectParameter;
                else
                    player.samplesPerTick = BITBOX_SAMPLERATE / (2 * effectParameter / 5);
                break;
        }

        // must recalculate channelfrequency ?
        if ( note != NONOTE || ( 
            player.lastAmigaPeriod[channel] &&
            effectNumber != VIBRATO && 
            effectNumber != VIBRATOVOLUMESLIDE &&
            !(effectNumber == 0xE && effectParameterX == NOTEDELAY) ) 
            )
            mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];

        if (note != NONOTE)
            mixer.channelSampleOffset[channel] = sampleOffset << DIVIDER;

        if (sampleNumber)
            mixer.channelSampleNumber[channel] = player.lastSampleNumber[channel];

        if (effectNumber != TREMOLO)
            mixer.channelVolume[channel] = player.volume[channel];

    }
}


void processTick() {
    uint16_t tempNote;

    for (uint8_t channel = 0; channel < player.numberOfChannels; channel++) {

        if (player.lastAmigaPeriod[channel]) {

            uint8_t sampleNumber = player.currentPattern.sampleNumber[player.lastRow][channel];
            uint16_t note = player.currentPattern.note[player.lastRow][channel];
            uint8_t effectNumber = player.currentPattern.effectNumber[player.lastRow][channel];
            uint8_t effectParameter = player.currentPattern.effectParameter[player.lastRow][channel];
            uint8_t effectParameterX = effectParameter >> 4;
            uint8_t effectParameterY = effectParameter & 0xF;
            
            switch(effectNumber) {
                case ARPEGGIO:
                    if (effectParameter)
                        switch(player.tick % 3) {
                            case 0:
                                mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];
                                break;
                            case 1:
                                tempNote = player.lastNote[channel] + effectParameterX * 8 + mod->samples[player.lastSampleNumber[channel]].fineTune;
                                if (tempNote < 296) mixer.channelFrequency[channel] = player.amiga / amigaPeriods[tempNote];
                                break;
                            case 2:
                                tempNote = player.lastNote[channel] + effectParameterY * 8 + mod->samples[player.lastSampleNumber[channel]].fineTune;
                                if (tempNote < 296) mixer.channelFrequency[channel] = player.amiga / amigaPeriods[tempNote];
                                break;
                        }
                    break;

                case PORTAMENTOUP:
                    player.lastAmigaPeriod[channel] -= effectParameter;
                    if (player.lastAmigaPeriod[channel] < 113) player.lastAmigaPeriod[channel] = 113;
                    mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];
                    break;

                case PORTAMENTODOWN:
                    player.lastAmigaPeriod[channel] += effectParameter;
                    if (player.lastAmigaPeriod[channel] > 856) player.lastAmigaPeriod[channel] = 856;
                    mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];
                    break;

                case TONEPORTAMENTO:
                    portamento(channel);
                    break;

                case VIBRATO:
                    vibrato(channel);
                    break;

                case PORTAMENTOVOLUMESLIDE:
                    portamento(channel);
                    player.volume[channel] += effectParameterX - effectParameterY;
                    if (player.volume[channel] < 0) player.volume[channel] = 0;
                    else if (player.volume[channel] > 64) player.volume[channel] = 64;
                    mixer.channelVolume[channel] = player.volume[channel];
                    break;

                case VIBRATOVOLUMESLIDE:
                    vibrato(channel);
                    player.volume[channel] += effectParameterX - effectParameterY;
                    if (player.volume[channel] < 0) player.volume[channel] = 0;
                    else if (player.volume[channel] > 64) player.volume[channel] = 64;
                    mixer.channelVolume[channel] = player.volume[channel];
                    break;

                case TREMOLO:
                    tremolo(channel);
                    break;

                case VOLUMESLIDE:
                    player.volume[channel] += effectParameterX - effectParameterY;
                    if (player.volume[channel] < 0) player.volume[channel] = 0;
                    else if (player.volume[channel] > 64) player.volume[channel] = 64;
                    mixer.channelVolume[channel] = player.volume[channel];
                    break;

                case 0xE:
                    switch(effectParameterX) {
                        case RETRIGGERNOTE:
                            if (!effectParameterY) break;
                            if ( !(player.tick % effectParameterY) ) {
                                mixer.channelSampleOffset[channel] = 0;
                            }
                            break;

                        case NOTECUT:
                            if (player.tick == effectParameterY)
                                mixer.channelVolume[channel] = player.volume[channel] = 0;
                            break;

                        case NOTEDELAY:
                            if (player.tick == effectParameterY) {
                                if (sampleNumber) player.volume[channel] = mod->samples[player.lastSampleNumber[channel]].volume;
                                if (note != NONOTE) mixer.channelSampleOffset[channel] = 0;
                                mixer.channelFrequency[channel] = player.amiga / player.lastAmigaPeriod[channel];
                                mixer.channelVolume[channel] = player.volume[channel];
                            }
                            break;
                    }
                    break;
            }

        }
    }
}


/* updates the player state : tick, rows or patterns */
void update_player() 
{
    if (player.tick == player.speed) {
        player.tick = 0;

        if (player.row == ROWS) {
            player.orderIndex++;
            if (player.orderIndex == mod->songLength)
                player.orderIndex = 0;
            player.row = 0;
        }

        if (player.patternDelay) {
            player.patternDelay--;
        } else {
            if (player.orderIndex != player.oldOrderIndex)
                loadPattern(mod->order[player.orderIndex]);
            player.oldOrderIndex = player.orderIndex;
            processRow();
        }

    } else {
        processTick();
    }
    player.tick++;
}

/* generates a sample from the current player status */
uint16_t gen_sample(uint16_t *buffer) 
{
    int16_t sumL = 0;
    #if MOD_STEREOSEPARATION!=64
    int16_t sumR = 0;
    #endif

    for (int channel = 0; channel < player.numberOfChannels; channel++) {
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
        out <<= BITDEPTH - 8;

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
    sumL /= player.numberOfChannels;
    sumR /= player.numberOfChannels;
    sample = ( sumL + (1 << (BITDEPTH-1)) )<<8 | ( ( sumR+ (1 << (BITDEPTH-1)) ) );
    #else // mono take left channel only
    sample = ((sumL / player.numberOfChannels)+128) * 0x0101;
    #endif 

    return sample;
}


void player_reset(void)
{
    player.amiga = AMIGA;
    player.samplesPerTick = BITBOX_SAMPLERATE / (2 * 125 / 5); // Hz = 2 * BPM / 5
    player.speed = 6;
    player.tick = player.speed;
    player.row = 0;

    player.orderIndex = 0;
    player.oldOrderIndex = 0xFF;
    player.patternDelay = 0;


    // reset player
    for (uint8_t channel = 0; channel < player.numberOfChannels; channel++) {
        player.patternLoopCount[channel] = 0;
        player.patternLoopRow[channel] = 0;

        player.lastAmigaPeriod[channel] = 0;

        player.waveControl[channel] = 0;

        player.vibratoSpeed[channel] = 0;
        player.vibratoDepth[channel] = 0;
        player.vibratoPos[channel] = 0;

        player.tremoloSpeed[channel] = 0;
        player.tremoloDepth[channel] = 0;
        player.tremoloPos[channel] = 0;

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

void load_mod(struct Mod *ldmod) 
{
    mod = ldmod; // global

    loadHeader();
    loadSamples();

    player_reset();
}

