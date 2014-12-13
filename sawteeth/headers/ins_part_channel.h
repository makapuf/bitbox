/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef INSPARTCHANNEL
#define INSPARTCHANNEL

#include "txt.h"
#include "types.h"

class InsStep
{
public:
	uint8 note;	//8bit note
	bool relative;		// 0-1 // kom ih√g att inte stoppa hit annat
	uint8 wform;		// max 15
};

class TimeLev
{
public:
	uint8 time;
	uint8 lev;
};

class Ins
{
public:
	Ins();

	void Free();
	
	status_t Load(txt &t);
	
	InsStep *steps;

	// Filter - Amp
	TimeLev *amp;
	TimeLev *filter;

	uint8 amppoints;
	uint8 filterpoints;

	uint8 filtermode;
	uint8 clipmode;
	uint8 boost;
	
	uint8 sps;	// PAL-screens per step	
	uint8 res;	// resonance

	uint8 vibs;
	uint8 vibd;
	
	uint8 pwms;
	uint8 pwmd;
	
   uint8 loop;
   uint8 len;	 
};


struct Step
{
	uint8 note;
	uint8 ins;
	uint8 eff;		//4bits eff 4bits value
};

class Part
{
public:
	Part();
	~Part();

	Step *steps;
	uint8 sps;		// PAL-screens per step	
	uint8 len;

};

struct ChStep
{
	uint8 part;
	int8 transp;
	uint8 damp;
};

class Channel
{
public:
	Channel();
	~Channel();
	
	uint8 left;
	uint8 right;

	uint16 len;
	uint16 lloop;
	uint16 rloop;
	
	ChStep *steps;
};

class BreakPoint
{
public:
	uint32	PAL;
	uint32	command;
};

#endif
