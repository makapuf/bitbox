#pragma once

#include <stdint.h>

#define BITBOX_SAMPLERATE 31469 
#define BITBOX_SNDBUF_LEN (BITBOX_SAMPLERATE/60+1) // 524 samples, 16ms

extern int audio_on; // will start when needed

void audio_init();
void audio_frame(); // write buffer should be written each frame, no special callback.

// user provided : fill a buffer with 8bit L/R sound data
void game_snd_buffer(uint16_t *buffer, int len); 


// ---------- small engine

typedef struct Sample
{
	// format + enum 
    unsigned int length; // size in number of samples
    unsigned int sample_rate; // sampling freq
    uint8_t *data; // stereo 2*8 bits
} Sample;

void audio_start_sample(Sample *s);
