/** Initialize the player to play the given song.
 * - Oscillators are reset to silence
 * - Song position is reset to 0
 * - Tracks and instruments are loaded from the song.
 *
 * Pass NULL to stop playing and not load a new song.
 */
/* Simple soundengine for the BitBox
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project */
#pragma once
#include <stdint.h>

/*
	Track is an 32 array of
		0NNNNNNN ccccCCCC pppppppp PPPPPPPP
		N : note (0 : no note, 7f : stop)
		c : command1 (0-f)
		C : command2 (0-f)
		p : parameter1 (0-ff)
		P : parameter2 (0-ff)
 
	 0 : no command
	 1: 0 = note off
	 2: d = duty cycle
	 3: f = volume slide
	 4: i = inertia (auto note slides)
	 5: j = instrument jump
	 6: l = slide
	 7: m = duty variation
	 8: t = timing (or song speed)
	 9: v = volume
	 10: w = select waveform
	 11: ~ = vibrato
	 12: + = set relative note
	 13: = = set absolute note (or instrument in pattern context)
	 14: ! = set instrument 


that means that only one command 'f' is available unless we differentiate for instruments and track commands)
*/

struct ChipSong {
	uint16_t songlen; // number of steps in the track sequencer
	uint8_t numchannels; // 4 or 8 channels supported by now (number of tracks which can play simultaneously)
	uint8_t tracklength; // 32 notes per track by default
	uint8_t *tracklist; // id of tracks. songlen * numchannels
	int8_t *transpose; // number of semitones. songlen * numchannels
	
	uint16_t **instruments; // pointer to an array of pointers to command<<8 | parameter

	uint32_t **tracks; // array of pointer to 32 u32 elements. 
};

// init with a song. use 0 to stop playing
void chip_play(const struct ChipSong *song); 

// play a note of this instrument now - useful for FX ! 
void chip_note(uint8_t ch, uint8_t note, uint8_t instrument);

int chip_song_playing();
