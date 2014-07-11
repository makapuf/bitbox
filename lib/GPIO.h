#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f4xx.h"

#include <stdint.h>

#define enable_gpio(PORT) RCC_AHB1ENR_## PORT ##EN


static inline uint32_t InterleaveZeros(uint32_t val)
{
	val=(val|(val<<8))&0x00ff00ff;
	val=(val|(val<<4))&0x0f0f0f0f;
	val=(val|(val<<2))&0x33333333;
	val=(val|(val<<1))&0x55555555;
	return val;
}

static inline uint32_t SetBits(uint32_t original,uint32_t bits,int state)
{
	if(!state) return original&~bits;
	else return original|bits;
}

static inline uint32_t SetDoubleBits(uint32_t original,uint16_t bits,int state)
{
	uint32_t mask=InterleaveZeros(bits);
	return (original&~(mask*3))|(mask*state);
}


static inline void SetGPIOInputMode(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->MODER=SetDoubleBits(gpio->MODER,pins,0);
}
static inline void SetGPIOOutputMode(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->MODER=SetDoubleBits(gpio->MODER,pins,1);
}
static inline void SetGPIOAlternateFunctionMode(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->MODER=SetDoubleBits(gpio->MODER,pins,2);
}
static inline void SetGPIOAnalogMode(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->MODER=SetDoubleBits(gpio->MODER,pins,3);
}

static inline void SetGPIOPushPullOutput(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OTYPER=SetBits(gpio->OTYPER,pins,0);
}
static inline void SetGPIOOpenDrainOutput(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OTYPER=SetBits(gpio->OTYPER,pins,1);
}

static inline void SetGPIOSpeed2MHz(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OSPEEDR=SetDoubleBits(gpio->OSPEEDR,pins,0);
}
static inline void SetGPIOSpeed25MHz(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OSPEEDR=SetDoubleBits(gpio->OSPEEDR,pins,1);
}
static inline void SetGPIOSpeed50MHz(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OSPEEDR=SetDoubleBits(gpio->OSPEEDR,pins,2);
}
static inline void SetGPIOSpeed100MHz(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->OSPEEDR=SetDoubleBits(gpio->OSPEEDR,pins,3);
}

static inline void SetGPIONoPullResistor(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->PUPDR=SetDoubleBits(gpio->PUPDR,pins,0);
}
static inline void SetGPIOPullUpResistor(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->PUPDR=SetDoubleBits(gpio->PUPDR,pins,1);
}
static inline void SetGPIOPullDownResistor(GPIO_TypeDef *gpio,uint16_t pins)
{
	gpio->PUPDR=SetDoubleBits(gpio->PUPDR,pins,2);
}

static inline void SelectAlternateFunctionForGPIOPin(GPIO_TypeDef *gpio,int pin,int function)
{
	int regindex=pin>>3;
	int position=(pin&0x07)*4;
	gpio->AFR[regindex]=(gpio->AFR[regindex]&~(0x0f<<position))|(function<<position);
}

#endif
