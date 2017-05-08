#pragma once

/*
   Original code by  "Pascal Piazzalunga" - http://www.serveurperso.com
   Adapted for bitbox by makapuf - makapuf2@gmail.com
 */


#include <stdbool.h>
#include <stdint.h>

#define MOD_ENABLE_LINEAR_INTERPOLATION

#define BITDEPTH BITBOX_SAMPLE_BITDEPTH // tyically 8
#define DIVIDER 10                      // Fixed-point mantissa used for integer arithmetic

#define MOD_STEREOSEPARATION 64                       // 0 (max) to 64 (mono) 

// Hz = 7093789 / (amigaPeriod * 2) for PAL
// Hz = 7159091 / (amigaPeriod * 2) for NTSC
#define AMIGA (7093789 / 2 / BITBOX_SAMPLERATE << DIVIDER)
// Mixer.channelFrequency[channel] = AMIGA / amigaPeriod

#define ROWS 64
#define SAMPLES 31
#define CHANNELS 4 // 4 ? 6 ? 32?
#define NONOTE 0xFFFF


struct Sample {
    uint8_t name[22];
    uint16_t length;
    int8_t fineTune;
    uint8_t volume;
    uint16_t loopBegin;
    uint16_t loopLength;
};

struct Mod {
    uint8_t name[20];
    struct Sample samples[SAMPLES];
    uint8_t songLength;
    uint8_t _dummy;
    uint8_t order[128];
    char tag[4];
};

struct Pattern {
    uint8_t sampleNumber[ROWS][CHANNELS];
    uint16_t note[ROWS][CHANNELS];
    uint8_t effectNumber[ROWS][CHANNELS];
    uint8_t effectParameter[ROWS][CHANNELS];
};

struct Player {
	// all precomputed
    struct Pattern currentPattern;

    uint8_t numberOfPatterns; 
    uint8_t numberOfChannels; 

    uint32_t amiga;
    uint16_t samplesPerTick;
    uint8_t speed;
    uint8_t tick;
    uint8_t row;
    uint8_t lastRow;

    uint8_t orderIndex;
    uint8_t oldOrderIndex;
    uint8_t patternDelay;
    uint8_t patternLoopCount[CHANNELS];
    uint8_t patternLoopRow[CHANNELS];

    uint8_t lastSampleNumber[CHANNELS];
    int8_t volume[CHANNELS];
    uint16_t lastNote[CHANNELS];
    uint16_t amigaPeriod[CHANNELS];
    int16_t lastAmigaPeriod[CHANNELS];

    uint16_t portamentoNote[CHANNELS];
    uint8_t portamentoSpeed[CHANNELS];

    uint8_t waveControl[CHANNELS];

    uint8_t vibratoSpeed[CHANNELS];
    uint8_t vibratoDepth[CHANNELS];
    int8_t vibratoPos[CHANNELS];

    uint8_t tremoloSpeed[CHANNELS];
    uint8_t tremoloDepth[CHANNELS];
    int8_t tremoloPos[CHANNELS];
};

struct Mixer {
    uint32_t sampleBegin[SAMPLES];
    uint32_t sampleEnd[SAMPLES];
    uint32_t sampleloopBegin[SAMPLES];
    uint16_t sampleLoopLength[SAMPLES];
    uint32_t sampleLoopEnd[SAMPLES];

    uint8_t channelSampleNumber[CHANNELS];
    uint32_t channelSampleOffset[CHANNELS];
    uint16_t channelFrequency[CHANNELS];
    uint8_t channelVolume[CHANNELS];
    uint8_t channelPanning[CHANNELS];
};

extern struct Player player;
extern struct Mod *mod;
extern struct Mixer mixer;


// External prototypes
void load_mod(struct Mod *mod);
void update_player(void);
uint16_t gen_sample (uint16_t *buffer);

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
