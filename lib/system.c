#include "system.h"

extern InterruptHandler *__isr_vector_sram[];
extern uint32_t __isr_vector_start[];

// Interrupt handlers.

static InterruptHandler **WritableInterruptTable();
void Default_Handler();


void InstallInterruptHandler(IRQn_Type interrupt,InterruptHandler handler)
{
	// TODO: Disable interrupts.
	InterruptHandler **currenttable=WritableInterruptTable();
	currenttable[interrupt+16]=handler;
}

void RemoveInterruptHandler(IRQn_Type interrupt,InterruptHandler handler)
{
	// TODO: Disable interrupts.
	InterruptHandler **currenttable=WritableInterruptTable();
	currenttable[interrupt+16]=Default_Handler;
}

static InterruptHandler **WritableInterruptTable()
{
	InterruptHandler **currenttable=(InterruptHandler **)SCB->VTOR;
	if((uint32_t)currenttable==(uint32_t)__isr_vector_start) 
	{
		InterruptHandler **flashtable=(InterruptHandler **)__isr_vector_start; 
		currenttable=__isr_vector_sram;
		for(int i=0;i<98;i++) currenttable[i]=flashtable[i];

		SCB->VTOR=(uint32_t)currenttable;
	}

	return currenttable;
}
