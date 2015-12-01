/* This is microX standard kernel configuration options. A standard dev should not modify it, 
 * as this one is used to tune clocks / overclocking, VGA clocks ... 
 */
#include <stdint.h>

#define STM32F401xE 
#define STACKSIZE 4096

// usb
#ifndef NO_USB
#define USE_USB_OTG_FS
#endif 

/*#define HSYNC_TIMER TIM3
#define HSYNC_INTERRUPT TIM3_IRQHandler
#define VIDEO_DAC_GPIO GPIOA
*/
 
#if defined (VGAMODE_NONE)

#elif defined(VGAMODE_400)
// 400x300 based on 800x600 + skipline 

#define VGA_H_PIXELS 400 
#define VGA_H_FRONTPORCH 16
#define VGA_H_SYNC 58
#define VGA_H_BACKPORCH 27

#define VGA_V_PIXELS 300
#define VGA_V_FRONTPORCH 2
#define VGA_V_SYNC 1
#define VGA_V_BACKPORCH 2

#define VGA_SKIPLINE
#define VGA_FPS 55
#define VGA_PIXELCLOCK 3 // DMA clocks per pixel

#else 

// Default : 320x240 / 60Hz (default) @ 84MHz

#define VGA_H_PIXELS 320 
#define VGA_H_FRONTPORCH 8
#define VGA_H_SYNC 29
#define VGA_H_BACKPORCH 24

#define VGA_V_PIXELS 240
#define VGA_V_FRONTPORCH 30
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 13

#define VGA_SKIPLINE
#define VGA_FPS 60 
#define VGA_PIXELCLOCK 6 // DMA clocks per pixel

#endif


// PLL global setup
// result:84MHz / ahb=84MHz / apb1=42MHz/ USB 48MHz

#define PLL_M 8
#define PLL_N 336
#define PLL_P 4
#define PLL_Q 7
#define AHB_PRE  RCC_CFGR_HPRE_DIV1
#define APB1_PRE RCC_CFGR_PPRE1_DIV2
// APB Timer is x2 if DIV=2,4,... (refman p.93)
#define APB1_DIV 1 
#define APB2_PRE RCC_CFGR_PPRE2_DIV1

