// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "bitbox.h"
#include "system.h" // interrupt handling

static uint16_t audio_buffer[BITBOX_SNDBUF_LEN*2]; // u16 stereo samples

static void DMAAudioCompleteHandler();

void audio_init()
{
	// clear buffer
	for (int i=0;i<BITBOX_SNDBUF_LEN*2;i++) 
		audio_buffer[i]=0;

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

	/* 
	  DMA setting : setting the DAC 1 and 2 with DMA implies : 
	  (also look at DM00049125, or 
	  https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f4-discovery/dac-dma/dac-dma.c )
	*/

	// Enable sample clock on timer 6
	// debug timer6 regs with p *(TIM_TypeDef*)(0x40001000)
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

	TIM6->PSC=0;  // Prescaler = 1 
	TIM6->ARR = SYSCLK/APB1_DIV/BITBOX_SAMPLERATE; // ~ 168MHz /2 / -> 65536 max !
	TIM6->CR1 = TIM_CR1_ARPE;	// autoreload preload enable, no other function on
	TIM6->CR2 = TIM_CR2_MMS_1; // MMS : TRGO Update mode 0b010

	TIM6->CR1 |= TIM_CR1_CEN; // go
	
	// -- DMA Setup 
    // Using DMA1 Streams 5 (DAC1) (or 6 - DAC2) on Channel 7 with the right 16/32 bits size dest (if 2x12 or 2x8 bits), 
  	// Using the DMA with circular buffers, with the 2 x buffer size 
	
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; // DMA1 enable

	// Stop it and configure interrupts.
    // Using the DMA Half & Full triggers to be notified that 1st half & second half are being transferred to fill the buffers
  	// debug DMA with p *((DMA_Stream_TypeDef *) 0x40026088)
	DMA1_Stream5->CR &= ~DMA_SxCR_EN;
	
	//NVIC_DisableIRQ(DMA1_Stream5_IRQn);
	// InstallInterruptHandler(DMA1_Stream5_IRQn,DMA1_Stream5_IRQHandler); // XXX make it a flash one
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	NVIC_SetPriority(DMA1_Stream5_IRQn,15);
	
	DMA1_Stream5->CR=(7*DMA_SxCR_CHSEL_0)| // Channel 7
		(1*DMA_SxCR_PL_0)| // Priority 1
		(1*DMA_SxCR_PSIZE_0)| // PSIZE = 16 bit
		(1*DMA_SxCR_MSIZE_0)| // MSIZE = 16 bit
		DMA_SxCR_MINC| // Increase memory address
		(1*DMA_SxCR_DIR_0)| // Memory to peripheral
		DMA_SxCR_HTIE |  // Half Transfer interrupt 
		DMA_SxCR_TCIE |  // Transfer complete interrupt 
		DMA_SxCR_CIRC |  // Circular buffer mode
		(1*DMA_SxCR_MBURST_0); // burst on the memory-side

	DMA1_Stream5->NDTR=BITBOX_SNDBUF_LEN*2; 
	DMA1_Stream5->PAR=((uint32_t)&(DAC->DHR8RD)); // L+R, 8Bits
	DMA1_Stream5->M0AR=(uint32_t)audio_buffer; 

	// Enable FIFO (see p190 of ref manual) (as MBURST/ref p236) ?
	// DMA1_Stream5->FCR |= DMA_SxFCR_DMDIS; 
	
	// Clear all Interrupt flags just in case (or it wont start)
	DMA1->HIFCR |= DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

	// Go
	DMA1_Stream5->CR |= DMA_SxCR_EN; 

    // Triggering the DAC from a Timer (timer6) with DAC_TIM6_TRGO - see refman 11.3.6
	// TEN1/2 for triggered value change. Set to TIM6TRGO, which is TSEL=000

	// Enabling the DAC as DMA source (only 1, or 2 requests will be provided) see refman 11.4.7	
	DAC->CR |= DAC_CR_TEN1 | DAC_CR_TEN2 | DAC_CR_DMAEN1; // clear DAC_CR_TSEL1 to select Tim6
	
	// enables DAC out. Automatically setup pin to DAC output
	DAC->CR |= DAC_CR_EN1 | DAC_CR_EN2; 
}

static void DMA1_Stream5_IRQHandler() {
	// Test which case : half or full ?

	// Clear Transfer complete interrupt flag of stream 5
	if (DMA1->HISR & DMA_HISR_TCIF5) {
		DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
		game_snd_buffer(&audio_buffer[BITBOX_SNDBUF_LEN],BITBOX_SNDBUF_LEN); // fill second half
	} else if (DMA1->HISR & DMA_HISR_HTIF5) {
		// Half full 
		DMA1->HIFCR |= DMA_HIFCR_CHTIF5;
		game_snd_buffer(audio_buffer,BITBOX_SNDBUF_LEN); // fill first half
	}
	// wait for effective clear ? No need to stop, DMA shall continue
}

// default empty implementation (silence)
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len) {}
