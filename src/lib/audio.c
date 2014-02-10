// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "audio.h"


// XXX use double buffering ? DMA ?
uint8_t audio_buffer[BITBOX_SNDBUF_LEN]; // one sample per line
uint8_t *audio_ptr; // current sample to play 
uint8_t audio_on;


void audio_init()
{
	audio_on = 0;
	
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

	audio_ptr = audio_buffer; 

}


void audio_frame()
{
	if (audio_on) {
		// XXX switch buffers
		audio_ptr = audio_buffer;
		game_snd_buffer(audio_buffer,BITBOX_SNDBUF_LEN); 
	}
}
void audio_out8(uint8_t value)
{
	// outputs value to DAC, value 0-255
	DAC->DHR8R1 = (uint32_t) value;
}

/*
int sample_id=0;
Sample *sample;



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

*/