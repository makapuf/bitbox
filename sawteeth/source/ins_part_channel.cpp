/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#include <stdlib.h>
#include <string.h>

#include "ins_part_channel.h"
#include "txt.h"
#include "st_version.h"
#include "stlimits.h"

Ins::Ins(){
	steps = 0;
	filter = 0;
	amp = 0;
};


void Ins::Free(){
	if (steps)	delete steps;
	if (amp) 	delete amp;
	if (filter) delete filter;
};


status_t Ins::Load(txt &t){
		filterpoints = t.NextVal();
		if (filterpoints < 1){ return JNG_ERROR;}

#ifdef TARGET_EDITOR
		filter = new TimeLev [ INSPOINTS ];
		memset((void*)filter,0,INSPOINTS * sizeof(TimeLev));
#else
		filter = new TimeLev [ filterpoints ];
#endif
		{
			for (int c1= 0; c1 < filterpoints; c1++){
				filter[c1].time =	t.NextVal();
				filter[c1].lev =		t.NextVal();
			}
		}

		amppoints = t.NextVal();
		if (amppoints < 1){ return JNG_ERROR;}

		amp = new TimeLev [ amppoints ];
		for (int c1 = 0; c1 < amppoints; c1++){
			amp[c1].time =	t.NextVal();
			amp[c1].lev =	t.NextVal();
		}
		
		filtermode = t.NextVal();
		clipmode = t.NextVal();
		boost = clipmode & 15;
		clipmode >>= 4;
		
		vibs = t.NextVal();
		vibd = t.NextVal();
		pwms = t.NextVal();
		pwmd = t.NextVal();
		res = t.NextVal();
		sps = t.NextVal();
		if (sps < 1){ return JNG_ERROR;}
		
#if 0
		if (st_version < 900){
			uint8 tmp = t.NextVal();
			len = tmp & 127;
			loop = (tmp & 1)?0:(len-1);
		}else{
#endif
			len = t.NextVal();
			loop = t.NextVal();
			if (loop >= len){ return JNG_ERROR;}
//		}

	if (len < 1){ return JNG_ERROR;}
	steps = new InsStep [ len ];

	for (int c1=0; c1 < len ; c1++){
		uint8 temp = t.NextVal();
		steps[c1].relative = (temp & 0x80) != 0;
		steps[c1].wform =		(temp & 0xf);
		steps[c1].note = 		t.NextVal();
	}

	return JNG_OK;
}


Channel::Channel(){};
Channel::~Channel(){};

