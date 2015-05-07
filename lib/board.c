// board.c : misc board elements definitions ; buttons, LED, debug blinks  ...
// interface in bitbox.h
#include "stm32f4xx.h"

void board_init()
{
	// User LED  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA
	GPIOA->MODER |= (1 << 4) ; // set pin 2 to be general purpose output

	// button is PE15
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // enable GPIO 
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR15_0; // set input / pullup 
	
	// SDIO sense is PC7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // enable GPIO 
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR7_0; // set input / pullup 

	// Profiling
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk ; // enable the cycle counter
}


void toggle_led()
{
	GPIOA->ODR ^= 1<<2; // led on/off
}

void set_led(int value)
{
	if (value)
		GPIOA->BSRRL |= 1<<2;
	else
		GPIOA->BSRRH |= 1<<2;
}

int button_state()
{
	return GPIOE->IDR & GPIO_IDR_IDR_15;
}

int sdio_sense_state()
{
	return GPIOC->IDR & GPIO_IDR_IDR_7;
}


void message(const char * msg, ...) {
	// does nothing on bitbox (UART ? write to flash ? to RAM )
}

// die : standard blinking to sgnal on the user led where we died and why

#define WAIT_TIME 168000000/128 // quick ticks, random number, good enough
void wait(int k) {
	for (volatile int i=0;i<k*WAIT_TIME;i++) {};
}

void blink(int times, int speed) {
	for (int i=0;i<times;i++) 
		{
			set_led(1);wait(speed);
			set_led(0);wait(speed);
		}
}

void die(int where, int cause)
{
	for (;;)
	{
		blink(where,3);
		wait(4);
		blink(cause,2);
		wait(4);
	}
}
