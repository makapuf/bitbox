/* This is bitbox standard kernel configuration options. A standard dev should not modify it, 
 * as this one is used to tune clocks / overclocking, VGA clocks ... 
 */
#include <stdint.h>

#define HAS_CMM 
#define STM32F405xx
#define STACKSIZE 8192
 
// usb
#ifndef NO_USB
#define USE_USB_OTG_HS 
#define USE_EMBEDDED_PHY 
#define USE_USB_OTG_FS 
#endif 

#define AHB_PRE RCC_CFGR_HPRE_DIV1
#define APB1_PRE RCC_CFGR_PPRE2_DIV2 // PCLK2 = HCLK / 2
#define APB2_PRE RCC_CFGR_PPRE1_DIV4 // PCLK1 = HCLK / 4
#define APB1_DIV 2

#if defined (VGAMODE_NONE)

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7

#elif defined(VGAMODE_640_OVERCLOCK)

// 640 480 VESA on 192 MHz SYSCLK

#define VGA_H_PIXELS 640 
#define VGA_V_PIXELS 480
#define VGA_FPS 60
 
#define VGA_H_FRONTPORCH 16 
#define VGA_H_SYNC 96
#define VGA_H_BACKPORCH 48

#define VGA_V_FRONTPORCH 10
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 33
#define VGA_V_PIXELS 480
#define VGA_V_BLANK 45

#define VGA_PIXELCLOCK 7 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 192L
#define PLL_P 2
#define PLL_Q 8

#elif defined(VGAMODE_800)

// 800 600 non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_H_PIXELS 800 
#define VGA_V_PIXELS 600
#define VGA_FPS 56

#define VGA_H_FRONTPORCH 64 
#define VGA_H_SYNC 128
#define VGA_H_BACKPORCH 50

#define VGA_V_FRONTPORCH 1
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 14

#define VGA_V_BLANK 19

#define VGA_PIXELCLOCK 5 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 360
#define PLL_P 4
#define PLL_Q 15

#elif defined(VGAMODE_800_OVERCLOCK)

// 800 600 non VESA O/C core (192Hz) - ~ 34kHz, 56fps
#define VGA_H_PIXELS 800 
#define VGA_V_PIXELS 600
#define VGA_V_BLANK 19
#define VGA_FPS 56

#define VGA_H_FRONTPORCH 32 
#define VGA_H_SYNC 80
#define VGA_H_BACKPORCH 50

#define VGA_V_FRONTPORCH 1
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 14

#define VGA_PIXELCLOCK 6 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 192
#define PLL_P 2
#define PLL_Q 8


#elif defined(VGAMODE_400)
// 400x300 based on 800x600 + skipline / non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_SKIPLINE
#define VGA_H_PIXELS 400 
#define VGA_V_PIXELS 300
#define VGA_V_BLANK 10
#define VGA_FPS 56


#define VGA_H_FRONTPORCH 32 
#define VGA_H_SYNC 64
#define VGA_H_BACKPORCH 25

#define VGA_V_FRONTPORCH 1
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 7

#define VGA_FPS 56
#define VGA_PIXELCLOCK 10 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 360
#define PLL_P 4
#define PLL_Q 15

#elif defined(VGAMODE_320)

// 320 240 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps
#define VGA_SKIPLINE
#define VGA_H_PIXELS 320 
#define VGA_V_PIXELS 240
#define VGA_V_FRONTPORCH 10
#define VGA_FPS 60

#define VGA_H_FRONTPORCH 8 
#define VGA_H_SYNC 48
#define VGA_H_BACKPORCH 24

#define VGA_V_FRONTPORCH 2
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 6

#define VGA_PIXELCLOCK 14 // DMA clocks per pixel

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7

#else // default one, use 640 480 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps

#define VGA_H_PIXELS 640 
#define VGA_V_PIXELS 480
#define VGA_V_BLANK 20
#define VGA_FPS 60

#define VGA_H_FRONTPORCH 16 
#define VGA_H_SYNC 64
#define VGA_H_BACKPORCH 80

#define VGA_V_FRONTPORCH 3
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 13

#define VGA_PIXELCLOCK 7 // DMA clocks per pixel

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7

#endif

