/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "insply.h"
#include "stSong.h"

class Player
{
public:
	Player(stSong *s, Channel *chn);
	~Player();

	// if buffer was updated
	bool	NextBuffer();
	float	*Buffer();

	bool Looped();
	
	void JumpPos(uint8 seqpos, uint8 steppos, uint8 pal);
private:

	bool looped;
	bool tmploop;
	
	InsPly *ip;
	stSong *song;
	ChStep *step;
	Channel *ch;
	
	Step *currstep;	
	Part *currpart;
	uint32 seqcount;		// step in sequencer
	uint8 stepc;		// step in part
	
	uint32 nexts;		// PAL-screen counter	

	float damp;
	float amp;
	float ampstep;	

	float freq;
	float freqstep;	
	float targetfreq;
	
	float cutoff;
	float cutoffstep;
	
	//------------------
	// buffers
	float *buffer;
};
#endif
