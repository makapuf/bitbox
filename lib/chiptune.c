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


#define MAX_CHANNELS 8
#include "bitbox.h"

#include <stdint.h>
#include <stdlib.h>
#include "chiptune.h"
 
int16_t songwait; // >0 means wait N frames, 0 means play now. <0 means stop playing now.
uint8_t trackpos;
uint8_t songspeed;
uint8_t playsong;
uint8_t nchan; // number of active channels
uint16_t songpos;

static const uint16_t freqtable[] = {
	0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165, 0x017a, 0x0191, 0x01a9,
	0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259, 0x027d, 0x02a3, 0x02cb,
	0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3, 0x042f, 0x046f, 0x04b2,
	0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5, 0x070a, 0x0775, 0x07e6,
	0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c, 0x0bd6, 0x0c8b, 0x0d4a,
	0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb, 0x13e9, 0x1518, 0x1659,
	0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b, 0x217c, 0x237a, 0x2596,
	0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528, 0x3851, 0x3bab, 0x3f37,
	0x42f9, 0x46f5, 0x4b2d, 0x4fa6, 0x5462, 0x5967, 0x5eb7, 0x6459, 0x6a51,
	0x70a3, 0x7756, 0x7e6f
};

static const int8_t sinetable[] = {
	0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
	127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
	0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
	-125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
	-71, -60, -49, -37, -25, -12
};


struct channel {
	uint8_t			tnum;
	int8_t			transp;
	uint8_t			tnote;
	uint8_t			lastinstr;
	uint8_t			inum;
	uint16_t		iptr;
	uint8_t			iwait;
	uint8_t			inote;
	int8_t			bendd;
	int16_t			bend;
	int8_t			volumed;
	int16_t			dutyd;
	uint8_t			vdepth;
	uint8_t			vrate;
	uint8_t			vpos;
	int16_t			inertia;
	uint16_t		slur;
} channel[MAX_CHANNELS];


// These are our possible waveforms. Any other value plays silence.
enum {
	WF_TRI, // triangle /\/\,
	WF_SAW, // sawteeth /|/|,
	WF_PUL, // pulse (adjustable duty) |_|-,
	WF_NOI // noise !*@?
};

// This is the definition of our oscillators. There are 8 of these (4 for left,
// 4 for right).
struct oscillator {
	uint16_t	freq; // frequency (except for noise, unused)
	uint16_t	phase; // phase (except for noise, unused)
	uint16_t	duty; // duty cycle (pulse wave only)
	uint8_t	waveform; // waveform (from the enum above)
	uint8_t	volume;	// 0-255
	uint8_t bitcrush; // 0-f level of quantization (power of 2)
};

// At each sample the phase is incremented by frequency/4. It is then used to
// compute the output of the oscillator depending on the waveform.
// This means the frequency unit is 65536*4/31000 or about 8.456Hz
// and the frequency range is 0 to 554180Hz. Maybe it would be better to adjust
// the scaling factor to allow less high frequencies (they are useless) but
// more fine grained resolution. Not only we could play notes more in tune,
// but also we would get a more subtle vibrato effect.

// ... and that's it for the engine, which is very simple as you see.
// The parameters for the oscillators can be updated in your game_frame callback.
// Since the audio buffer is generated in one go it is useless to try to tweak
// the parameters more often than that.


struct oscillator osc[MAX_CHANNELS];


struct ChipSong *current_song;

static void runcmd(uint8_t ch, uint8_t cmd, uint8_t param, uint8_t context) {
	// context is 0 for instrument, 1/2 for pattern
	switch(cmd) {
		case 1: // 0 = note off
			channel[ch].inum = 0;
			break;
		case 2: // d = duty cycle
			osc[ch].duty = param << 8;
			break;
		case 3: // f = volume slide
			channel[ch].volumed = param;
			break;
		case 4: // i = inertia (auto note slides)
			channel[ch].inertia = param << 1;
			break;
		case 5: // j = instrument jump
			channel[ch].iptr = param;
			break;
		case 6: // l = slide
			channel[ch].bendd = param;
			break;
		case 7: // m = duty variation
			channel[ch].dutyd = param << 6;
			break;
		case 8: // t = timing (for instrument, song speed for track context)
			if (!context)
				channel[ch].iwait = param;
			else 
				songspeed = param;
			break;
		case 9: // v = volume
			osc[ch].volume = param;
			break;
		case 10: // w = select waveform
			osc[ch].waveform = param;
			break;
		case 11: // ~ = vibrato
			if(channel[ch].vdepth != (param >> 4)) {
				channel[ch].vpos = 0;
			}
			channel[ch].vdepth = param >> 4;
			channel[ch].vrate = param & 15;
			break;
		case 12: // + = set relative note 
			channel[ch].inote = param + channel[ch].tnote - 12 * 4;
			break;
		case 13: // = = set absolute note in instrument context or instrument in pattern context
			if (!context)
				channel[ch].inote = param;
			else 
				channel[ch].lastinstr = param;
			break;
		case 14: 
			osc[ch].bitcrush=param;
	}
}

void chip_play(const struct ChipSong *song) {
	current_song = (struct ChipSong*) song;
	if (!song) { // if given NULL, just stop it now
		playsong=0;
		return;
	}
	nchan = current_song->numchannels; // number of channels

	songwait = 0;
	trackpos = 0;
	playsong = 1;
	songpos = 0;
	songspeed=4; // default speed

	for (int i=0;i<nchan;i++) {
		osc[i].volume = 0;
		channel[i].inum = 0;
		osc[i].bitcrush = 5;
	}
}

void chip_note(uint8_t ch, uint8_t note, uint8_t instrument) 
{
	channel[ch].tnote = note ;
	channel[ch].inum = instrument;
	channel[ch].iptr = 0;
	channel[ch].iwait = 0;
	channel[ch].bend = 0;
	channel[ch].bendd = 0;
	channel[ch].volumed = 0;
	channel[ch].dutyd = 0;
	channel[ch].vdepth = 0;
}

static void chip_song_update()
// this shall be called each 1/60 sec. 
// one buffer is 512 samples @32kHz, which is ~ 62.5 Hz,
// calling each song frame should be OK
{
	if(songwait) {
		songwait--;
	} else {
		songwait = songspeed; 

		if(!trackpos) {
			if(playsong) {
				if(songpos >= current_song->songlen) {
					playsong = 0;
					message("Stopping song\n");
				} else {
					message("Now at position %d of song\n",songpos);
					for(int ch = 0; ch < nchan; ch++) {
						// read each of the track pattern
						channel[ch].tnum = current_song->tracklist[songpos*nchan+ch];
						channel[ch].transp = current_song->transpose[songpos*nchan+ch];
					}
					songpos++;
				}
			}
		}

		if(playsong) {
			message ("%d |",trackpos);
			for(int ch = 0; ch < nchan; ch++) { 
				if (!channel[ch].tnum) 
					continue;
				
				uint32_t fields = current_song->tracks[channel[ch].tnum-1][trackpos];

				// field = x:1 note:7 cmd1:4 cmd2:4 par1:8 par2:8
				uint8_t note = (fields>>24) & 0x7f;
				uint8_t cmd1 = (fields>>20) & 0xf;
				uint8_t cmd2 = (fields>>16) & 0xf;
				uint8_t par1 = (fields>>8) & 0xff;
				uint8_t par2 = (fields>>0) & 0xff;
				message("Note:%02x %x %02x / %x %02x |",note, cmd1,par1,cmd2,par2);

				if(cmd1) runcmd(ch, cmd1, par1,1);
				if(cmd2) runcmd(ch, cmd2, par2,2);

				if(note) 
					chip_note(ch,note + channel[ch].transp, channel[ch].lastinstr);

			}
			message("\n");

			trackpos++;
			if (trackpos == current_song->tracklength)
				trackpos = 0;
		}
	}
}

static void chip_osc_update()
{
	for(int ch = 0; ch < nchan; ch++) {
		int16_t vol;
		uint16_t duty;
		uint16_t slur;

		while(channel[ch].inum && !channel[ch].iwait) {
			// run instrument instruction iptr from instr inum  as cmd, param
			uint16_t ins = current_song->instruments[channel[ch].inum-1][channel[ch].iptr];

			channel[ch].iptr++;

			runcmd(ch, ins>>8, ins & 0xff,0);
		}

		if(channel[ch].iwait) 
			channel[ch].iwait--;

		if(channel[ch].inertia) {
			int16_t diff;

			slur = channel[ch].slur;
			diff = freqtable[channel[ch].inote] - slur;
			//diff >>= channel[ch].inertia;
			if(diff > 0) {
				if(diff > channel[ch].inertia) diff = channel[ch].inertia;
			} else if(diff < 0) {
				if(diff < -channel[ch].inertia) diff = -channel[ch].inertia;
			}
			slur += diff;
			channel[ch].slur = slur;
		} else {
			slur = freqtable[channel[ch].inote];
		}
		osc[ch].freq =
			slur +
			channel[ch].bend +
			((channel[ch].vdepth * sinetable[channel[ch].vpos & 63]) >> 2);
		channel[ch].bend += channel[ch].bendd;
		vol = osc[ch].volume + channel[ch].volumed;
		if(vol < 0) vol = 0;
		if(vol > 255) vol = 255;
		osc[ch].volume = vol;

		duty = osc[ch].duty + channel[ch].dutyd;
		if(duty > 0xe000) duty = 0x2000;
		if(duty < 0x2000) duty = 0xe000;
		osc[ch].duty = duty;

		channel[ch].vpos += channel[ch].vrate;
	}
}


// This function generates one audio sample for all 8 oscillators. The returned
// value is a 2*8bit stereo audio sample ready for putting in the audio buffer.
static inline uint16_t gen_sample()
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
	for(int i=0; i<nchan; i++) {
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
	if (nchan == 4)
		return (128 + (acc[0] >> 7)) | ((128 + (acc[1] >> 7)) << 8);	// [1,251]
	else
		return (128 + (acc[0] >> 8)) | ((128 + (acc[1] >> 8)) << 8);	// [1,251]
}

void game_snd_buffer(uint16_t* buffer, int len) {
	if (current_song) {
		if (playsong)
			chip_song_update();
			// even if there's no song, update oscillators in case a "chip_note" gets called.
		chip_osc_update(); 
	}
	// Just generate enough samples to fill the buffer.
	for (int i = 0; i < len; i++) {
		buffer[i] = gen_sample();
	}
}

int chip_song_playing()
{
    return (playsong != 0);
}
