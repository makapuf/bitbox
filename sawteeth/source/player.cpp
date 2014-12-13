/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

//#include <stdio.h>
#include "insply.h"
#include "player.h"
//#include "stSong.h"

Player::Player(stSong *s, Channel *chn)
{
	looped = false;
	tmploop = false;
	amp = freq = freqstep = cutoff = cutoffstep = 1.0;	
	ampstep = 0.0;

	ip=new InsPly(s);
	song=s;
	step = chn->steps;
	ch = chn;		
	JumpPos(0,0,0);	

	buffer = new float[s->spspal];
}

Player::~Player()
{
	delete ip;
	delete buffer;
}

void Player::JumpPos(uint8 seqpos, uint8 steppos, uint8 pal)
{
	stepc = steppos;
	seqcount = seqpos;
	nexts = 0;

	currpart = &(song->parts[step[seqcount].part]);
	currstep =& (currpart->steps[stepc]);

	damp = ((float)255.0 - (float)step[seqcount].damp) / (float)255.0;		

	if (pal > 0){
		// increase counters;
		stepc++;
		if (stepc >= currpart->len){
			stepc=0;
			seqcount++;
			if (seqcount >= ch->len) seqcount=0;	// limit
			currpart=&(song->parts[step[seqcount].part]);
		}

		nexts = currpart->sps - pal;
		currstep =& (currpart->steps[stepc]);
	}
	
}

bool Player::NextBuffer()
{
	if (nexts < 1){
		damp = ((float)255.0 - (float)step[seqcount].damp) / (float)255.0;		
		nexts=currpart->sps;
		
		currstep=&(currpart->steps[stepc]);
		if (currstep->ins){
			ip->TrigADSR(currstep->ins);
			amp = 1.0;
			ampstep = 0;
			cutoff = 1.0;
			cutoffstep = 1.0;
		}

		if (currstep->note){
			targetfreq = song->N2F[currstep->note+ch->steps[seqcount].transp];
			freqstep = 1.0;
			if ((currstep->eff & 0xf0) != 0x30)freq = targetfreq;
		}
		
		
		// EFFEKTER //////////////////////////////////////////////////////////////////////////////
		const float divider = 50.0;
		if (currstep->eff){
			switch (currstep->eff & 0xf0){

			// pitch
			case 0x10 :	{	targetfreq = 44100;	freqstep = 1+(((currstep->eff & 0xf)+1)/(divider*3));	} break;
			case 0x20 :	{	targetfreq = 1;		freqstep = 1-(((currstep->eff & 0xf)+1)/(divider*3));	} break;
			case 0x30 :	{
				// dubbelt arbete ... targetfreq = song->N2F[currstep->note+ch->steps[seqcount].transp];
				if ( targetfreq > freq )
					freqstep = 1+(((currstep->eff & 0xf)+1)/divider);
				else
					freqstep = 1-(((currstep->eff & 0xf)+1)/divider);
			} break;

			// PWM
			case 0x40 : ip->SetPWMOffs((currstep->eff & 0xf)/(float)16.1); break;
			
			// resonance
			case 0x50 : ip->SetReso((currstep->eff & 0xf)/(float)15.0); break;

			// filter
			case 0x70 :	cutoffstep = (float)1.0-((currstep->eff & 0xf)+1)/(float)256.0;break;
			case 0x80 :	cutoffstep = (float)1.0+((currstep->eff & 0xf)+1)/(float)256.0;break;
			case 0x90 :
				{
					cutoffstep = (float)1.0;
					cutoff = ((currstep->eff & 0xf)+1)/(float)16.0;
					cutoff *= cutoff;
				} break;

			// amp
			case 0xa0 :	ampstep = -((currstep->eff & 0xf)+1)/(float)256.0; break;
			case 0xb0 :	ampstep = ((currstep->eff & 0xf)+1)/(float)256.0; break;
			case 0xc0 : ampstep=0; amp = (currstep->eff & 0xf)/(float)15.0; break;

			}
		}

		// increase counters;
		if (tmploop){
			looped = true;
			tmploop = false;
		} else looped = false;

		stepc++;
		if (stepc >= currpart->len){
//			printf("0. seqcount %d,  ch->len %d\n",seqcount, ch->len);
			stepc=0;
			seqcount++;

			if (seqcount > ch->rloop){
				seqcount = ch->lloop;	// limit
				tmploop = true;
//				printf("LOOOPED !!!!\n");
			}

//			printf("1. seqcount %d,  ch->len %d\n",seqcount, ch->len);

			currpart=&(song->parts[step[seqcount].part]);
		}
//		printf("step = %d\n", stepc);
	}
	nexts--;

	cutoff *= cutoffstep;
	if (cutoff<0){
		cutoff=0;
		cutoffstep = 1.0;
	}else{
		if (cutoff>1){
		cutoff = 1;
		cutoffstep = 1.0;		
		}
	}

	freq *= freqstep;
	if (freqstep > 1.0001){
		if (freq > targetfreq){
			freq=targetfreq;
			freqstep=1;
		}	
	}else{
		if (freqstep < 0.9999)
			if (freq < targetfreq){
				freq=targetfreq;
				freqstep=1;
			}	
	}
	
	amp += ampstep;
	if (amp<0){
		amp=0;
		ampstep=0;
	}else{
		if (amp>1){
		amp=1;
		ampstep=0;		
		}
	}

	ip->SetAmp(amp  * damp );
	ip->SetFreq(freq);
	ip->SetCutOff(cutoff);

	// spela upp
	return ip->Next( buffer, song->spspal );
}

float *Player::Buffer()
{
	return buffer;
}

bool Player::Looped(){
	if (looped){
		looped = false;
		return true;
	}else
		return false;
	
}

