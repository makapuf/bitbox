#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stm32f4xx.h"

typedef void InterruptHandler();

void system_init();

void InstallInterruptHandler(IRQn_Type interrupt,InterruptHandler handler);
void RemoveInterruptHandler(IRQn_Type interrupt,InterruptHandler handler);

static inline void EnableInterrupt(IRQn_Type interrupt)
{
	int regindex=interrupt>>5;
	int shift=interrupt&0x1f;
	NVIC->ISER[regindex]|=1<<shift;
}

static inline void DisableInterrupt(IRQn_Type interrupt)
{
	int regindex=interrupt>>5;
	int shift=interrupt&0x1f;
	NVIC->ISER[regindex]&=~(1<<shift);
}

static inline void SetInterruptPriority(IRQn_Type interrupt,int priority)
{
	NVIC->IP[interrupt]=priority<<4;
}

#endif
