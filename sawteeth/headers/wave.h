/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef WAVE_H
#define WAVE_H

#include "types.h"

enum {	HOLD = 0,
		SAW	= 1,
		SQR = 2,
		TRI = 3,
		NOS = 4,
		SIN = 5,
		
		TRIU = 6,
		SINU = 7 
};


class Wave
{
public:
	Wave(float freq, uint8 waveform);
	Wave();

	// returns if buffer is updated
	bool Next(float *buffer, uint32 count);

	void SetFreq(	float freq);
	void SetPWM(	float p);
	void SetForm(	uint8 waveform);
	void SetAmp(	float a);

	void NoInterp();

private:
	void FillSaw(	float *buffer, uint32 count, float amp);
	void FillSquare(float *buffer, uint32 count, float amp);
	void FillTri(	float *buffer, uint32 count, float amp);
	void FillNoise(	float *buffer, uint32 count, float amp);
	void FillSin(	float *buffer, uint32 count, float amp);

	void FillTriU(	float *buffer, uint32 count, float amp);
	void FillSinU(	float *buffer, uint32 count, float amp);

	uint8 form;
	bool _pwm_lo;
	float _amp,fromamp;

	float pwm;		//	mellan 0 och 1

	float currval;	//	räknare mellan -1 och 1 var i cykeln man är
	
	float curr;
	float step;

	float noiseval;
	
	uint32 sincurrval;	//	räknare mellan 0 och 0xFFFFFFFF 
	uint32 sinstep;

	// SQR & TRI-STUFF
	float sqrval;	//	amp eller -amp
	float fstep;	//	2*step || -2*step
	float limit;	//	antingen pwm eller 1

};

#endif
