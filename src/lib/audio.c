// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "audio.h"

void audio_init()
{
	// enable DAC clock on APB1
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	// enable GPIOA clock on APB2
	RCC->APB2ENR |= RCC_AHB1LPENR_GPIOALPEN;

	/* Configure PA.04 (DAC) as Analog */

	// Set GPIOA pin X as ANALOG
	GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER4_1;

	// Useful ? not that fast.
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4_1; // 50Mhz = 10 set bit 1
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR4_0); // 50Mhz = 10 reset bit 0

	// enables DAC out. Automatically setup pin to DAC output
	DAC->CR = DAC_CR_EN1; 
	// clear TEN1 for immediate value change
}

int sample_id=0;
Sample *sample;

void audio_out8(uint8_t value)
{
	// outputs value to DAC, value 0-255
	DAC->DHR8R1 = (uint32_t) value;
}


// ultra simple 1-voice, non tuned, non looped sampler
void audio_start_sample(Sample *s)
{
	sample=s;
	sample_id=0;
}

void audio_play_sample()
{
	if (!sample) return;
	// every other sample
	audio_out8(sample->data[sample_id++/2]); 

	if (sample_id>2*sample->length) 
	{
		audio_out8(0);
		sample = 0;
	}
}

void audio_tri1k() 
{
	audio_out8(8*(sample_id++)&31);
}

