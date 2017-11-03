/*
TODO : 
	- acceleration of mixing func
	- read data from WAV (u8, mono only) see http://stackoverflow.com/questions/14948442/code-to-read-wav-header-file-producing-strange-results-c
			(or embed wavs)

	- vol enveloppes ?
	- interpolation
	- note stealing
	- dither
	- synth voices ?
	- funny small noises when playing on bitbox itself (?)
	- slow tempo ?
	- voices can be cut (see line 172)
*/
#include <stdint.h>
#include <string.h> 
#include <math.h> // powf

#include "bitbox.h"
#include "sampler.h"

struct Voice {
	int8_t *data; // buffer
	uint32_t data_len;  
	uint32_t data_pos; // current position in data/buffer *256.
	int32_t data_loop; // offset from start. -1 : dont loop.

	uint16_t speed; // output samples per input sample, *256. 0x100 = normal speed

	uint8_t vol_left, vol_right; // 0-255 per channel if 0,0 sample not used (free)
};

struct Sampler {
	uint32_t ticks; // buffers played so far
	struct Voice voices[MAX_VOICES];
};

struct Player {
	// Instrument
	const int8_t *sound_data;
	int data_len;
	int sound_loop;
	int c4speed; // sampling speed of C4 * 256
	// LR master volumes ? 

	// Events
	int nb_events;
	const struct NoteEvent *events;
	int tempo; // in buffers

	// Running info
	int note_pos; // index of next note played
	int note_tick; // next note played, in buffers
	int8_t playing_notes[128]; // [note_id] ->  voice_id ou 0xff : currently playing notes 
};

// ----------------------------------------------------------
static struct Sampler s;
static struct Player player;

// ----------------------------------------------------------
static inline int is_free(int voice_id) 
{
	return !s.voices[voice_id].vol_left && !s.voices[voice_id].vol_right;
}

static inline int find_free_voice()
{
	for (int i=0;i<MAX_VOICES;i++)
		if (is_free(i)) return i;
	return -1;
}


int play_sample(const int8_t *data, int data_len, uint16_t speed, int loop_pos, uint8_t vol_left, uint8_t vol_right)
{
	int idx=find_free_voice();
	if (idx>=0) {
		struct Voice *v = &s.voices[idx];
		v->data=(int8_t*) data; // non-const
		v->data_len=data_len*256;
		v->vol_left=vol_left;
		v->vol_right=vol_right;
		v->speed = speed;
		v->data_loop = loop_pos*256;
		
		// rewind
		v->data_pos=0; 
	}
	return idx;
}

void stop_sample(int sample_id)
{
	s.voices[sample_id].vol_right=s.voices[sample_id].vol_left=0;
}

void stop_all_samples()
{
	for (int sample_id=0;sample_id<MAX_VOICES;sample_id++)
		stop_sample(sample_id);
}

void player_step(uint32_t ticks);

void game_snd_buffer(uint16_t *buffer, int len)
{
	
	// MIX voices into buffer.
	uint8_t *buffer8 = (uint8_t*)buffer;

	for (int i=0;i<len*2;i++)
		buffer8[i]=128;

	for (int vi=0;vi<MAX_VOICES;vi++) 	
	{		
		if (is_free(vi)) continue; // skip

		struct Voice *v;
		v = &s.voices[vi];

		int nb = (v->data_len-v->data_pos)*256/v->speed; // number of samples available in memory, as output samples
		if (len<nb) nb=len; 

		// mixing available data to buffer
		for (int i=0;i<nb;v->data_pos+=v->speed,i++) {
			// FIXME use assembly / SIMD instrs !

			int8_t smp = v->data[v->data_pos>>8]; // XXX linear interp

			int8_t a = (smp*v->vol_left)>>8;
			int8_t b = (smp*v->vol_right)>>8;
			buffer8[i*2] +=  a;
			buffer8[i*2+1] +=  b;
		};

		// end of memory / filebuffer ?
		if (v->data_pos>=v->data_len) {
			// end of sample : loop ?
			if (v->data_loop<0) {
				// end of sample
				v->vol_left = v->vol_right = 0;
			} else {
				v->data_pos = v->data_loop;
			} // XXX pingpong: speed = -speed, ...

		} 
	}

	// Play current track
	if (player.note_pos < player.nb_events) 
		player_step(s.ticks);

	s.ticks++;
} 

void player_step(uint32_t ticks) 
{
	struct NoteEvent note;
	while (ticks >= player.note_tick) { 
		// process all arrived events
		note = player.events[player.note_pos++];
		player.note_tick = ticks + 60 * BITBOX_SAMPLERATE * player.events[player.note_pos].tick / player.tempo / BITBOX_SNDBUF_LEN / 24; // next one, in beats per ticks

		if (note.note>0) { // <0 are special ones
			// interrupt note playing if exists (for new note or note off)
			int voice_id = player.playing_notes[note.note];
			if ( voice_id != -1 ) {
				stop_sample(voice_id); // XXX what if already stopped and reused by another note ? 
				player.playing_notes[note.note]=-1; // erase data;
			}

			if (note.vel>0) { 
				// note on
				int speed = (int)(player.c4speed * powf(1.059463094f, note.note - 60));
				player.playing_notes[note.note] = play_sample(player.sound_data, player.data_len, speed, player.sound_loop, note.vel, note.vel);
			}

		} 
	}
}

void play_track (int nb_events, int tempo, const struct NoteEvent *events, const int8_t *sound_data, int sound_loop, int data_len, int c4freq)
{
	player.sound_data = sound_data;
	player.data_len = data_len;
	player.c4speed = 256.f * c4freq / BITBOX_SAMPLERATE;
	player.sound_loop = sound_loop;

	player.nb_events=nb_events;
	player.events = events;
	player.tempo = tempo;

	player.note_pos = 0;
	player.note_tick = 0;
	
	for (int i=0;i<128;i++)
		player.playing_notes[i]=0xff; // OFF
}

void stop_track(void) 
{
	player.nb_events = 0;
	player.note_pos = 0;
}