#include "Random.h"

#include <stdbool.h>

static uint32_t s1=0xc7ff5f16,s2=0x0dc556ae,s3=0x78010089;

void SeedRandom(uint32_t seed)
{
	s1=(seed*1664525+1013904223)|0x10;
	s2=(seed*1103515245+12345)|0x1000;
	s3=(seed*214013+2531011)|0x100000;
}

void SeedRandom64(uint64_t seed)
{
	uint32_t a=seed>>32,b=(uint32_t)seed;

	s1=(a*1664525+1013904223)|0x10;
	s2=(b*1103515245+12345)|0x1000;
	s3=((a^b)*214013+2531011)|0x100000;
}

uint32_t RandomInteger()
{
	s1=((s1&0xfffffffe)<<12)^(((s1<<13)^s1)>>19);
	s2=((s2&0xfffffff8)<<4)^(((s2<<2)^s2)>>25);
	s3=((s3&0xfffffff0)<<17)^(((s3<<3)^s3)>>11);
	return s1^s2^s3;
}

uint32_t RandomIntegerInRange(uint32_t low,uint32_t high)
{
	return low+RandomInteger()%(high-low+1);
}

float RandomFloat()
{
	return (float)RandomInteger()/4294967296.0f;
}

double RandomDouble()
{
	return (double)RandomInteger()*(1.0/4294967296.0); 
}

double PreciseRandomDouble()
{
	uint32_t a=RandomInteger()>>5,b=RandomInteger()>>6; 
	return (a*67108864.0+b)/9007199254740992.0; 
}
