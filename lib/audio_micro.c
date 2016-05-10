// audio generation through PWM
#include <stdint.h>
#include "stm32f4xx.h"
#include "bitbox.h"

static uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // u8 mono samples.
// Note : it should be really uint8_t audiobuf[LEN] (mono samples, simple buffering)
// but to keep the same interface in bitbox.h we keep this declaration.

static uint16_t *audio_ptr=audio_buffer;

#define PWM_CYCL 256
// debug : p *((TIM_TypeDef *) (0x40000000UL)) because TIM2_BASE=APB1PERIPH_BASE + 0 !

void audio_init()
{
	// initialize PWM with audio on PB10
	// PB10 is toggled by TIM2 CH3.

	// configure GPIO Pin
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // enable clock

    GPIOB->MODER  |= GPIO_MODER_MODER10_0 * 0b10 ; // PB10:ALT
    GPIOB->AFR[1] |= 1<<8; // PB10 alt func nb 1 (datasheet p45+refman p162)
    GPIOB->PUPDR  |= GPIO_PUPDR_PUPDR10_0 * 0b01 ; // PB10 pullup 01
    GPIOB->OSPEEDR |= 0b11<<20; // SPEED=50MHz

    // Timer 2 global setup

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    // enable timer 2 clock

    TIM2->CR1=TIM_CR1_ARPE;  // autoreload preload enable, no other function on
    TIM2->CCER=0; // Disable CC, so we can program it.
    TIM2->ARR=PWM_CYCL-1;

    // Channel 3  : Pulse PWM

    // set to PWM mode active level on reload, passive level after CC3-match + enable preload
    TIM2->CCMR2|=6*TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3PE;
    // output enabled, reversed polarity (active low).
    TIM2->CCER|=TIM_CCER_CC3E|TIM_CCER_CC3P;

    TIM2->CNT=0; // make sure it hits ARR
    TIM2->CCR3=128; // mid-value for now
    TIM2->CR1 |= TIM_CR1_CEN; // Go !
}

/* on bitbox micro, audio is currently sent by the video output in hsync */
void audio_line( void ) {
	TIM2->CCR3 = (*audio_ptr++) & 0xff; // keep left channel
}

void audio_frame( void ) {
	// generate data for the next frame. No need to double buffering since we do it synchronously
	game_snd_buffer(audio_buffer,BITBOX_SNDBUF_LEN+1);
	audio_ptr=audio_buffer;
}

// default empty implementation (silence)
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len) {}
