/* board_micro : driver for micro */

#include "kconf.h"
#include "cmsis/stm32f4xx.h"

uint8_t __isr_vector_sram [0x200];

void set_led(int value)
{
    if (value)
        GPIOB->BSRR |= GPIO_BSRR_BR_0; // PB0
    else
        GPIOB->BSRR |= GPIO_BSRR_BS_0;
}


int button_state()
{
	return 0;
}

int sdio_sense_state()
{
	return 0;
}

void setup_clocks();

// misc setups : flash, fpu, profiling, LED
void system_init(void) 
{

    // Setup flash latency : 2WS + prefetch ON, I/C caches enabled
    SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_2WS | \
    	FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN ); 

	// enable hard FPU
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));    /* set CP10 and CP11 Full Access */

	// Enable Profiling
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk ; // enable the cycle counter

	setup_clocks();
}

// default empty implementation 
__attribute__((weak)) void audio_init ( void ) {}

void board_init(void)
{

	// LED init
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	// 00 IN, 01 OUT, 10 ALT, 11 ANALOG x 16 bits
	GPIOB->MODER |= 0b01 ; // PB0 as output  
	// output : push pull is default , Speed is 50M (0->3 ), default slow. 
}

// Setup Clocks (from default reset state)
void setup_clocks(void)
{
    SET_BIT(RCC->CR,RCC_CR_HSEON);  // switch hse on 
    while (!READ_BIT(RCC->CR, RCC_CR_HSERDY));  // wait till RCC_CR_HSE Ready 
    
    // system PLL from HSE (value from kconf.h, HSE=8Mhz)
    // SRC = HSE 8MHz. M=8, N=336, P=4 Q=7 => 84MHz clock
    
    RCC->PLLCFGR = (RCC_PLLCFGR_PLLM_0*PLL_M) | \
    	(RCC_PLLCFGR_PLLN_0*PLL_N) | \
    	(RCC_PLLCFGR_PLLP_0 * ((PLL_P-2)/2)) | \
    	(RCC_PLLCFGR_PLLQ_0 * PLL_Q) | \
    	RCC_PLLCFGR_PLLSRC_HSE;

    // Prescaling : AHB=1, APB1=2, APB2=1 (kconf)
    RCC->CFGR = AHB_PRE | APB1_PRE | APB2_PRE;

    SET_BIT(RCC->CR,RCC_CR_PLLON); 
    while (!READ_BIT(RCC->CR, RCC_CR_PLLRDY));  // wait till OK
    
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);  // Use PLL
    while (!READ_BIT(RCC->CFGR, RCC_CFGR_SWS_PLL)); // wait till done
}

// die : standard blinking to signal on the user led where we died and why

void message(const char * msg, ...) {
	// does nothing on bitbox (UART ? write to flash ? to RAM )
}

#define WAIT_TIME SYSCLK/128 // quick ticks, random number, good enough
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

