/* Simple soundengine for the BitBox
 * Copyright 2015, Makapuf <makapuf2@gmail.com>
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project
 *
 * This main file is a player for the packed music format used in the original
 * "hardware chiptune"
 * http://www.linusakesson.net/hardware/chiptune.php
 * There is a tracker for this but the format here is slightly different (mostly
 * because of the different replay rate - 32KHz instead of 16KHz).
 *
 * Because of this the sound in the tracker will be a bit different, but it can
 * easily be tweaked. This version has a somewhat bigger but much simplified song format.
 */
#include "chiptune.h"

struct oscillator osc[MAX_CHANNELS];

// This function generates one audio sample for all 8 oscillators. The returned
// value is a 2*8bit stereo audio sample ready for putting in the audio buffer.
//
// You most likely want to call this from game_snd_buffer to fill the buffers.
uint16_t gen_sample()
{
	int16_t acc[2]; // accumulators for each channel
	static uint32_t noiseseed = 1;

	// This is a simple noise generator based on an LFSR (linear feedback shift
	// register). It is fast and simple and works reasonably well for audio.
	// Note that we always run this so the noise is not dependant on the
	// oscillators frequencies.
	uint32_t newbit;
	newbit = 0;
	if(noiseseed & 0x80000000L) newbit ^= 1;
	if(noiseseed & 0x01000000L) newbit ^= 1;
	if(noiseseed & 0x00000040L) newbit ^= 1;
	if(noiseseed & 0x00000200L) newbit ^= 1;
	noiseseed = (noiseseed << 1) | newbit;

	acc[0] = 0;
	acc[1] = 0;
	// Now compute the value of each oscillator and mix them
	for(int i=0; i < MAX_CHANNELS; i++) {
		int8_t value; // [-32,31]

		switch(osc[i].waveform) {
			case WF_TRI:
				// Triangle: the part before 0x8000 raises, then it goes back
				// down.
				if(osc[i].phase < 0x8000) {
					value = -32 + (osc[i].phase >> 9);
				} else {
					value = 31 - ((osc[i].phase - 0x8000) >> 9);
				}
				break;
			case WF_SAW:
				// Sawtooth: always raising.
				value = -32 + (osc[i].phase >> 10);
				break;
			case WF_PUL:
				// Pulse: max value until we reach "duty", then min value.
				value = (osc[i].phase > osc[i].duty)? -32 : 31;
				break;
			case WF_NOI:
				// Noise: from the generator. Only the low order bits are used.
				value = (noiseseed & 63) - 32;
				break;
			default:
				value = 0;
				break;
		}
		// Compute the oscillator phase (position in the waveform) for next time
		osc[i].phase += osc[i].freq / 4;

		// bit crusher effect
		if (osc[i].bitcrush) {
			value |= ((1<<osc[i].bitcrush) - 1);
		}

		// Mix it in the appropriate output channel
		acc[i & 1] += value * osc[i].volume; // rhs = [-8160,7905]
	}
	// Now put the two channels together in the output word
	// acc [-32640,31620] > ret 2*[1,251]

	if (MAX_CHANNELS == 4)
		return (128 + (acc[0] >> 7)) | ((128 + (acc[1] >> 7)) << 8);    // [1,251]
	else
		return (128 + (acc[0] >> 8)) | ((128 + (acc[1] >> 8)) << 8);    // [1,251]
}

