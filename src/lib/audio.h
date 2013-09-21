#pragma once

#include <stdint.h>

typedef struct Sample
{
    unsigned int length; // size in number of samples
    unsigned int sample_rate; // sampling freq
    uint8_t *data;
} Sample;


void audio_init();
void audio_out8(uint8_t value);

// ultra simple mono sample player
void audio_start_sample(Sample *);
void audio_play_sample();

/* sample values */
void audio_tri1k();

