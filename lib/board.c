// board.c : misc board elements definitions ; buttons, LED, debug blinks  ...
// interface in bitbox.h

#include "kconf_bitbox.h"
#include "stm32f4xx.h"

extern uint32_t __isr_vector_start[]; // flash vector


static void setup_clocks();

void system_init()
{
	// Reset the RCC clock configuration to the default reset state
	RCC->CR|=0x00000001; // Set HSION bit
	RCC->CFGR=0x00000000;	// Reset CFGR register
	RCC->CR&=0xFEF6FFFF; // Reset HSEON, CSSON and PLLON bits
	RCC->PLLCFGR=0x24003010; // Reset PLLCFGR register
	RCC->CR&=0xFFFBFFFF; // Reset HSEBYP bit
	RCC->CIR=0x00000000; // Disable all interrupts

	// Configure the System clock source, PLL Multiplier and Divider factors, 
	// AHB/APBx prescalers and Flash settings
	setup_clocks();

	// Set vector table offset to flash memory start. This is the new (after bootloader) one
	SCB->VTOR=(uint32_t) __isr_vector_start; 

	// Set up interrupts to 4 bits preemption priority.
	SCB->AIRCR=0x05FA0000|0x300;
}


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


static void setup_clocks()
{
	// PLL (clocked by HSE) used as System clock source.

	// Enable HSE and wait until it is read.
	RCC->CR|=RCC_CR_HSEON;
	while(!(RCC->CR&RCC_CR_HSERDY));

	// Enable high performance mode, System frequency up to 168 MHz.
	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_PMODE;  

	RCC->CFGR|=RCC_CFGR_HPRE_DIV1; // HCLK = SYSCLK / 1
	RCC->CFGR|=RCC_CFGR_PPRE2_DIV2; // PCLK2 = HCLK / 2
	RCC->CFGR|=RCC_CFGR_PPRE1_DIV4; // PCLK1 = HCLK / 4

	// Configure the main PLL.
	RCC->PLLCFGR=PLL_M|(PLL_N<<6)|(((PLL_P>>1)-1)<<16)|(RCC_PLLCFGR_PLLSRC_HSE)|(PLL_Q<<24);

	// Enable the main PLL and wait until it is ready.
	RCC->CR|=RCC_CR_PLLON;
	while(!(RCC->CR&RCC_CR_PLLRDY));
   
	// Configure Flash prefetch, Instruction cache, Data cache and wait state
	FLASH->ACR=FLASH_ACR_ICEN|FLASH_ACR_DCEN|FLASH_ACR_LATENCY_5WS;

	// Select the main PLL as system clock source.
	RCC->CFGR&=~RCC_CFGR_SW;
	RCC->CFGR|=RCC_CFGR_SW_PLL;

	// Wait until the main PLL is used as system clock source.
	while((RCC->CFGR&RCC_CFGR_SWS)!=RCC_CFGR_SWS_PLL);
}

void toggle_led()
{
	GPIOA->ODR ^= 1<<2; // led on/off
}

void set_led(int value)
{
    if (value)
        GPIOA->BSRR |= GPIO_BSRR_BR_2; // PA2
    else
        GPIOA->BSRR |= GPIO_BSRR_BS_2;
}


int button_state()
{
	return !(GPIOE->IDR & GPIO_IDR_IDR_15);
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
