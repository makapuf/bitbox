// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "audio.h"

// TODO : use DMA (double buffering NOT CIRCULAR)
// see https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f4-discovery/dac-dma/dac-dma.c

static uint16_t audio_buffer1[BITBOX_SNDBUF_LEN]; // u16 stereo samples
static uint16_t audio_buffer2[BITBOX_SNDBUF_LEN]; 

uint16_t *audio_ptr; // current sample to play 
uint16_t *audio_read, *audio_write; // current sample to play 

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

	audio_read = &audio_buffer1[0];
	audio_write = &audio_buffer2[0];

	audio_ptr = audio_read; 

}

void audio_frame()
{
	if (audio_on) {
		// Switch buffers
		uint16_t * d = audio_write;
		audio_write = audio_read;
		audio_read = d;

		// assumes frame takes place exactly when needed . ?
		audio_ptr = audio_read;
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

// default empty implementation
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len)  {}
