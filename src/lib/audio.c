// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "audio.h"


// XXX use double buffering ? DMA ?
// explicit the rate !
uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // one sample per line
uint16_t *audio_ptr; // current sample to play 
int audio_on;


void audio_init()
{
	audio_on = 0;
	
	// enable DAC clock on APB1
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	// enable GPIOA clock on APB2
	RCC->APB2ENR |= RCC_AHB1LPENR_GPIOALPEN;

	/* Configure PA.04 & PA.05 (DAC1) as Analog */

	// Set GPIOA pin 4&5 as ANALOG
	GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER4_1;
	GPIOA->MODER |= GPIO_MODER_MODER5_0 | GPIO_MODER_MODER5_1;

	// Useful ? not that fast.
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4_1; // 50Mhz = 10 set bit 1
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR4_0); // 50Mhz = 10 reset bit 0
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_1; // 50Mhz = 10 set bit 1
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR5_0); // 50Mhz = 10 reset bit 0

	// enables DAC out. Automatically setup pin to DAC output
	// clear TEN1/2 for immediate value change
	DAC->CR |= DAC_CR_EN1 | DAC_CR_EN2 ; 


	audio_ptr = &audio_buffer[0]; 

}

void audio_frame()
{
	if (audio_on) {
		// XXX switch buffers
		audio_ptr = audio_buffer;
		game_snd_buffer(audio_buffer,BITBOX_SNDBUF_LEN); 
	}
}

void audio_out8(uint16_t valueLR)   
{
	// outputs value to DAC, value 0-255 <<8 | 0-255
	DAC->DHR8RD = (uint32_t) valueLR;
}

void audio_out12(uint32_t valueLR) // use 0-4095 values   
{
	// outputs value to DAC, value 0-4095 <<16 | 0-4095
	DAC->DHR12RD = (uint32_t) valueLR;
}



