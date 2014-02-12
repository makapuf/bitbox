/* This is the (old) SNES controller. 
probably not working now but could work quite easily by 
- replacing pins tp UEXT port 
- porting it to new Gamepad API - whatever this is.
*/

#include "gamepad.h"
#include "stm32f4xx.h"

#include "GPIO.h"
#include "RCC.h"

#define LATCH (1<<10) 
#define DATA (1<<11)
#define CLK (1<<12)
#define DATA_BIT 11

// public
volatile uint16_t gamepad1;

// private
static uint16_t next_value;
static int step;

void gamepad_init()
{
	EnableAHB1PeripheralClock(RCC_AHB1ENR_GPIOAEN);

	SetGPIOOutputMode(GPIOA,LATCH | CLK);
	SetGPIOInputMode(GPIOA,DATA);

	SetGPIOSpeed50MHz(GPIOA,LATCH | CLK | DATA);
	SetGPIOPullUpResistor(GPIOA,DATA);

	//SetGPIOPullUpResistor(GPIOA,LATCH | CLK | DATA);
	step = 0;

}

inline void gamepad_readstep()
{
	/*XXX convert to switch */
	// static int ?

	if (step<32)
	{
		if (step==0)
		{
			next_value=0;
			// latch/clock set low
			GPIOA->BSRRH |= (CLK|LATCH);			
		}
		if (step==1)
		{
		 	// latch set high
		 	GPIOA->BSRRL |= LATCH;
		}
		else if (step%2==0)
		{
			GPIOA->BSRRH |= LATCH; // set latch low
			next_value <<= 1;
			next_value |= ((GPIOA->IDR & DATA)>>DATA_BIT); // read data bit 1/0 as bit 1
			// clock set high
			GPIOA->BSRRL |= CLK;
		}
		else 
		{
			// clock low
			GPIOA->BSRRH |= CLK;
		}
		step++;
	} else if (step==32)//33 ?
	{
		gamepad1 =(~next_value)>>2;
		step=0;
	}
}

