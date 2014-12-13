/* Simple soundengine for the BitBox
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project
 */

#include <stdint.h>

// These are our possible waveforms. Any other value plays silence.
enum {
	WF_TRI, // triangle /\/\,
	WF_SAW, // sawteeth /|/|,
	WF_PUL, // pulse (adjustable duty) |_|-,
	WF_NOI // noise !*@?
};

// This is the definition of our oscillators. There are 4 of these (2 for left,
// 2 for right).
struct oscillator {
	uint16_t	freq; // frequency (except for noise, unused)
	uint16_t	phase; // phase (except for noise, unused)
	uint16_t	duty; // duty cycle (pulse wave only)
	uint8_t	waveform; // waveform (from the enum above)
	uint8_t	volume;	// 0-255
};

// At each sample the phase is incremented by frequency/4. It is then used to
// compute the output of the oscillator depending on the waveform.
// This means the frequency unit is 65536*4/31000 or about 8.456Hz
// and the frequency range is 0 to 554180Hz. Maybe it would be better to adjust
// the scaling factor to allow less high frequencies (they are useless) but
// more fine grained resolution. Not only we could play notes more in tune,
// but also we would get a more subtle vibrato effect.

extern volatile struct oscillator osc[4];

// ... and that's it for the engine, which is very simple as you see.
// The parameters for the oscillators can be updated in your game_frame callback.
// Since the audio buffer is generated in one go it is useless to try to tweak
// the parameters more often than that.
