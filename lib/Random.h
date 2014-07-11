#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdint.h>

void SeedRandom(uint32_t seed);
void SeedRandom64(uint64_t seed);
uint32_t RandomInteger();
uint32_t RandomIntegerInRange(uint32_t low,uint32_t high);
float RandomFloat();
double RandomDouble();
double PreciseRandomDouble();

#endif

