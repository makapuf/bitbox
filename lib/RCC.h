#ifndef __RCC_H__
#define __RCC_H__

#include "stm32f4xx.h"

#include <stdint.h>

static inline void EnableAHB1PeripheralClock(uint32_t peripherals) { RCC->AHB1ENR|=peripherals; }
static inline void EnableAHB2PeripheralClock(uint32_t peripherals) { RCC->AHB2ENR|=peripherals; }
static inline void EnableAHB3PeripheralClock(uint32_t peripherals) { RCC->AHB3ENR|=peripherals; }
static inline void EnableAPB1PeripheralClock(uint32_t peripherals) { RCC->APB1ENR|=peripherals; }
static inline void EnableAPB2PeripheralClock(uint32_t peripherals) { RCC->APB2ENR|=peripherals; }

static inline void DisableAHB1PeripheralClock(uint32_t peripherals) { RCC->AHB1ENR&=~peripherals; }
static inline void DisableAHB2PeripheralClock(uint32_t peripherals) { RCC->AHB2ENR&=~peripherals; }
static inline void DisableAHB3PeripheralClock(uint32_t peripherals) { RCC->AHB3ENR&=~peripherals; }
static inline void DisableAPB1PeripheralClock(uint32_t peripherals) { RCC->APB1ENR&=~peripherals; }
static inline void DisableAPB2PeripheralClock(uint32_t peripherals) { RCC->APB2ENR&=~peripherals; }

static inline void SetAHB1PeripheralReset(uint32_t peripherals) { RCC->AHB1RSTR|=peripherals; }
static inline void SetAHB2PeripheralReset(uint32_t peripherals) { RCC->AHB2RSTR|=peripherals; }
static inline void SetAHB3PeripheralReset(uint32_t peripherals) { RCC->AHB3RSTR|=peripherals; }
static inline void SetAPB1PeripheralReset(uint32_t peripherals) { RCC->APB1RSTR|=peripherals; }
static inline void SetAPB2PeripheralReset(uint32_t peripherals) { RCC->APB2RSTR|=peripherals; }

static inline void ClearAHB1PeripheralReset(uint32_t peripherals) { RCC->AHB1RSTR&=~peripherals; }
static inline void ClearAHB2PeripheralReset(uint32_t peripherals) { RCC->AHB2RSTR&=~peripherals; }
static inline void ClearAHB3PeripheralReset(uint32_t peripherals) { RCC->AHB3RSTR&=~peripherals; }
static inline void ClearAPB1PeripheralReset(uint32_t peripherals) { RCC->APB1RSTR&=~peripherals; }
static inline void ClearAPB2PeripheralReset(uint32_t peripherals) { RCC->APB2RSTR&=~peripherals; }

static inline void EnableAHB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB1LPENR|=peripherals; }
static inline void EnableAHB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB2LPENR|=peripherals; }
static inline void EnableAHB3PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB3LPENR|=peripherals; }
static inline void EnableAPB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB1LPENR|=peripherals; }
static inline void EnableAPB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB2LPENR|=peripherals; }

static inline void DisableAHB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB1LPENR&=~peripherals; }
static inline void DisableAHB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB2LPENR&=~peripherals; }
static inline void DisableAHB3PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB3LPENR&=~peripherals; }
static inline void DisableAPB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB1LPENR&=~peripherals; }
static inline void DisableAPB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB2LPENR&=~peripherals; }

static const uint8_t APBAHBPrescalerTable[16]={0,0,0,0,1,2,3,4,1,2,3,4,6,7,8,9};

static inline uint32_t SYSCLKFrequency()
{
	switch(RCC->CFGR&RCC_CFGR_SWS)
	{
		case 0x00:  // HSI used as system clock source
			return HSI_VALUE;

		default:
		case 0x04:  // HSE used as system clock source
			return HSE_VALUE;

		case 0x08: // PLL used as system clock source
		{
      		uint32_t srcclock;
			if(RCC->PLLCFGR&RCC_PLLCFGR_PLLSRC) srcclock=HSE_VALUE; // HSE used as PLL clock source
			else srcclock=HSI_VALUE; // HSI used as PLL clock source

			uint32_t pllm=RCC->PLLCFGR&RCC_PLLCFGR_PLLM;
			uint32_t pllvco=(srcclock/pllm)*((RCC->PLLCFGR&RCC_PLLCFGR_PLLN)>>6);
			uint32_t pllp=(((RCC->PLLCFGR&RCC_PLLCFGR_PLLP)>>16)+1)*2;
			return pllvco/pllp;
		}
	}
}

static inline uint32_t HCLKFrequency()
{
	uint32_t shift=APBAHBPrescalerTable[(RCC->CFGR&RCC_CFGR_HPRE)>>4];
	return SYSCLKFrequency()>>shift;
}

static inline uint32_t PCLK1Frequency()
{
	uint32_t shift=APBAHBPrescalerTable[(RCC->CFGR&RCC_CFGR_PPRE1)>>10];
	return SYSCLKFrequency()>>shift;
}

static inline uint32_t PCLK2Frequency()
{
	uint32_t shift=APBAHBPrescalerTable[(RCC->CFGR&RCC_CFGR_PPRE2)>>13];
	return SYSCLKFrequency()>>shift;
}

#endif
