/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef INSPLY_H
#define INSPLY_H

#include "types.h"
#include "wave.h"
#include "lfo.h"

class stSong;
class Ins;
class InsStep;

class InsPly
{
public:
	InsPly(stSong *s);
	bool Next(float *buffer, uint32 count);
	void TrigADSR(uint8 i);
	void SetPWMOffs(float a);
	void SetReso(float a);
	void SetAmp(float a);
	void SetFreq(float f);
	void SetCutOff(float co);
private:


//	filter
	void SLP(float *b, uint32 count);
	void OLP(float *b, uint32 count);
	void LP(float *b, uint32 count);
	void HP(float *b, uint32 count);
	void BP(float *b, uint32 count);
	void BS(float *b, uint32 count);
	void VanillaClip(float *b, uint32 count, float mul);
	void SinusClip(float *b, uint32 count, float mul);

//	clipping
	lfo *vib,*pwm;
	float vamp,pamp;
	float pwmoffs;
	
	stSong *song;
	Wave *w;
	Ins *ins;
	InsStep *currstep;
	uint8 stepc;		//instrument step
	int32 nexts;	//next ins

	Ins *currins;
	InsStep *steps;

	// Amp ADSR 
	bool trigged;
	float curramp;
	float ampstep;
	int8 adsr;
	int32 nextadsr;	

	// Filter ADSR 
	float currf;
	float fstep;
	int8 fadsr;
	int32 nextfadsr;	

	// from inspattern
	float insfreq;
	
	// from player
	float currpartfreq;
	float currpartamp;
	float currpartco;
	
	// curr
	float res;
	float amp;
	float cutoff;

	// for filter
	float lo,hi,bp,bs;

};

#endif
