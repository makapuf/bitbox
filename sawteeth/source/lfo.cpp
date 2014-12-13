/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#include "lfo.h"
#include <math.h>

lfo::lfo()
{
	curr = 0;
	step = (float)(6.28 * .00002267573696145124);
}

lfo::lfo(float freq)
{
	curr = 0;
	step = freq * (float)(6.28 * .00002267573696145124);
}

void lfo::SetFreq(float freq)
{
	step = freq * (float)(6.28 * .00002267573696145124);
}

float lfo::Next()
{
	return cosf( curr += step );
}
