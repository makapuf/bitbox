/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#include <stdio.h>
#include <math.h>
#include "wave.h"

#ifndef PI
#define PI (float)3.14159265
#endif

#define ABS(x) ((x)>0)?(x):(-x)
/////////////////////////////////////////
//	prototypes
unsigned long jng_rand();
/////////////////////////////////////////
//	own random-function
#define BIGMOD 0x7fffffff
#define W 127773
#define C 2836
long jng_seed=1;

unsigned long jng_rand()
{
	jng_seed = 16807*(jng_seed % W) - (jng_seed/W)*C;
	if (jng_seed < 0) jng_seed+=BIGMOD;
	return jng_seed;
}

// static table and initiation
static float sint[513]; //latter to optimize interpolation
static float trit[513]; //latter to optimize interpolation
static bool sisinit = false;
static void sinit()
{
	if (sisinit) return;

	// sinetable
	int c = 0;
	for (; c < 513 ; c++){
		sint[c] =
			sinf(
			(float)c *
			((float)2. * PI) /
			(float)512.0
			);
	}

	// tritable
	float smp = -1;
	float add = (float)4.0/(float)512.0;
	for (c = 0 ; c < 256 ; c++){
		trit[c] = smp;
		smp += add;
	}
	for (; c < 512 ; c++){
		trit[c] = smp;
		smp -= add;
	}
	trit[512] = trit[0];
	
	sisinit = true;
}

Wave::Wave(float freq, uint8 waveform)
{
	sinit();
	fromamp = noiseval = curr = pwm = step = sqrval = currval = 0.0;
	limit = 1.0;
	_pwm_lo = false;

	SetForm(waveform);
	SetFreq(freq);
}

Wave::Wave()
{
	sinit();
	fromamp = noiseval = curr = pwm = step = sqrval = currval = 0.0;
	limit = 1.0;
	_pwm_lo = false;

	SetForm(SAW);
	SetFreq(440);
}


void Wave::SetFreq(float freq)
{
	// den skumma konstanten Ã¤r 1/44100
	step = freq * (float).00002267573696145124;
	if (step > 1.9){
		step = (float)1.9;
	}
	
	sinstep = (uint32)( (float)(1<<22) * (float)512.0 * step);
}

const float pwmlim = (float)0.9;
void Wave::SetPWM(float p)
{
	if (p > pwmlim)
    	pwm = pwmlim;
	else if (p < -pwmlim)
		pwm = -pwmlim;
	else
		pwm = p;
		    
}

void Wave::SetAmp(float a)
{
	_amp = a;
}

void Wave::SetForm(uint8 waveform){
	if (form != waveform)
	{
		if (currval > 1) currval = 0.0;
		if (curr > 1) curr = 0.0;
	}
	
	form = waveform;
}

void Wave::NoInterp()
{
	fromamp = _amp;
}

bool Wave::Next(float *buffer, uint32 count)
{
	if (_amp < 0.001) return false;

	switch (form){
	case SAW:	FillSaw(   buffer, count, _amp); break;
	case SQR:	FillSquare(buffer, count, _amp); break;
	case NOS:	FillNoise( buffer, count, _amp); break;
	case TRI:	FillTri(   buffer, count, _amp); break;
	case SIN:	FillSin(   buffer, count, _amp); break;
	case TRIU:	FillTriU(  buffer, count, _amp); break;
	case SINU:	FillSinU(  buffer, count, _amp); break;
	default: return false;
	}

	return true;
}

// denna passar att unrolla
inline void Wave::FillSin(float *b, uint32 count, float amp)
{
	float astep = (amp-fromamp)/(float)count;
	amp = fromamp;
	while (count --){
		sincurrval += sinstep;
		
		uint32 pos = sincurrval >> 23; // plocka fram heltalsdelen
		float dec = (float)(sincurrval & ((1<<23)-1)) / (float)((1<<23)-1); // brÃ¥kdelen
		
		float val0 = sint[pos];
		float val1 = sint[pos+1];
		float val = ( val1 * dec ) + ( val0 * ((float)1.0 - dec));
		*b = amp * val;
		b++;
		amp+=astep;
	}
	fromamp = amp;
	
}

inline void Wave::FillSaw(float *out, uint32 count, float amp)
{
	float *stop = out+count;
	float _amp_add = (amp-fromamp)/(float)count;
	amp = fromamp;

	while (out < stop){
			// kolla om walk skall korrigeras
			if (curr >= 1)
			{
				// d Ã¤r var pÃ¥ samplen curr blev stÃ¶rre Ã¤n 1
				float d = (curr - 1) /(step);
				curr -= 2.0;

				*out = amp * (-2*d +1);
				out++;
				amp += _amp_add;
				curr += step;
			}
			// antalet samples kvar till _walk blir fel
			float walkdiff = 1.0 - curr;
			int walksteps = (int)((walkdiff/ step)+1);

			// antales samples kvar i buffern
			int steps = stop-out;
			
			// använd det minsta talet
			if (steps > walksteps) steps = walksteps;

			while (steps > 8)
			{
				out[0] = amp * curr; amp += _amp_add; curr += step;
				out[1] = amp * curr; amp += _amp_add; curr += step;
				out[2] = amp * curr; amp += _amp_add; curr += step;
				out[3] = amp * curr; amp += _amp_add; curr += step;
				out[4] = amp * curr; amp += _amp_add; curr += step;
				out[5] = amp * curr; amp += _amp_add; curr += step;
				out[6] = amp * curr; amp += _amp_add; curr += step;
				out[7] = amp * curr; amp += _amp_add; curr += step;
				out +=8;
				steps -= 8;
			}

			while (steps--)
			{
				*out = amp * curr;
				out ++;
				amp += _amp_add;
				curr += step;
			}
		}
	fromamp = amp;
}

inline void Wave::FillSquare(float *out, uint32 count, float amp)
{
	float _amp_add = (amp-fromamp)/(float)count;
	amp = fromamp;

	float *stop = out+count;

	while (out < stop){
			// kolla om walk skall korrigeras (anti-alias)
			if (curr >= 1.0)
			{
				float d = (curr - 1.0) /(step);

/*				if (curr > 1.5)
				{
					printf("0. curr==%f\n",curr);
				}
*/
				curr -= 1.0;
				if (_pwm_lo)
				{
					*out = amp*d +((1-d)* -amp);
					curr -= pwm;
				}else{
					*out = -amp*d +((1-d)*amp);
					curr += pwm;
				}
/*
				if (curr > 1.0)
				{
					printf("1. curr==%f\n",curr);
					return;
				}
*/
				_pwm_lo = !_pwm_lo;
				out++;

				amp += _amp_add;
				curr += step;
			}

			// antalet samples kvar till _walk blir fel
			float walkdiff = 1.0 - curr;
			int walksteps = (int)((walkdiff/ step)+1);

			// antales samples kvar i buffern
			int steps = stop-out;
			
			// använd det minsta talet
			if (steps > walksteps) steps = walksteps;

			// beräkna nya prylar
			float tmp_amp;
			float tmp_amp_add;
			if (_pwm_lo)
			{
				tmp_amp = -amp;
				tmp_amp_add = -_amp_add;
			}else{
				tmp_amp = amp;
				tmp_amp_add = _amp_add;
			}

			int counter = steps;
			
			while (counter--)
			{
				*out = tmp_amp;
				tmp_amp += tmp_amp_add;
				out ++;
			}

			// räkna upp räknarna
			amp  += steps * _amp_add;
			curr += steps * step;

	}
	fromamp = amp;
}

inline void Wave::FillNoise(float *out, uint32 count, float amp)
{
	float *stop = out+count;
	float _amp_add = (amp-fromamp)/(float)count;
	amp = fromamp;


	while (out < stop){			// kolla om walk skall korrigeras
			if (curr >= 1.0)
			{
				curr -= 2.0;
				noiseval = amp * ((jng_rand()/(float)(64.0*256.0*256.0*256.0))-(float)1);
			}

			// antalet samples kvar till _walk blir fel
			float walkdiff = 1.0 - curr;
			int walksteps = (int)((walkdiff/ step)+1);

			// antales samples kvar i buffern
			int steps = stop-out;
			
			// använd det minsta talet
			if (steps > walksteps) steps = walksteps;

			int counter = steps;
			
			while (counter--)
			{
				*out = noiseval;
				out ++;
			}
				curr += steps * step;
				amp  += steps * _amp_add;
		}
	fromamp = amp;

	if (curr >= 1.0) curr -= 2.0;
}

// this could be optimized
inline void Wave::FillTri(float *b, uint32 count, float amp)
{
	float astep = (amp-fromamp)/(float)count;
	amp = fromamp;
	
	float *stop = b + count;
	while (b < stop){
		*b = amp * (float)((float)2 * (float)((currval> 0 )?(-currval):(currval))+1.0);
		currval += step;
		if (currval>1.0) currval-=2.0;
		b++;

		amp+=astep;
	}
	fromamp = amp;
}



// ugly
inline void Wave::FillSinU(float *b, uint32 count, float amp)
{
	float astep = (amp-fromamp)/(float)count;
	amp = fromamp;
	while (count --){
		sincurrval += sinstep;
		
		uint32 pos = sincurrval >> 23; // plocka fram heltalsdelen
		*b = amp * sint[pos];
		b++;
		amp+=astep;
	}
	fromamp = amp;
	
}

inline void Wave::FillTriU(float *b, uint32 count, float amp)
{
	float astep = (amp-fromamp)/(float)count;
	amp = fromamp;
	while (count --){
		sincurrval += sinstep;
		
		uint32 pos = sincurrval >> 23; // plocka fram heltalsdelen
		*b = amp * trit[pos];
		b++;
		amp+=astep;
	}
	fromamp = amp;	
}


