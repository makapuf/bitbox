/* This is bitbox standard kernel configuration options. A standard dev should not modify it,
 * as this one is used to tune clocks / overclocking, VGA clocks ...
 */
#include <stdint.h>

#ifndef VGA_MODE
#define VGA_MODE 640
#endif 
#ifndef VGA_BPP
#define VGA_BPP 16
#endif 

#ifdef __MACH__
// for a Mac OS build, they want attributes specified differently:
#define CCM_MEMORY __attribute__ ((used, section ("__DATA, .ccm")))
#else
#define CCM_MEMORY __attribute__ ((section (".ccm")))
#endif

#define STM32F40_41xxx
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


// Video Mode
#if (VGA_MODE==NONE)

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7

#elif VGA_MODE==OVERCLOCK_640

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
#define VGA_V_BLANK 45

#define VGA_PIXELCLOCK 7 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 192L
#define PLL_P 2
#define PLL_Q 8

#elif VGA_MODE==800

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

#elif VGA_MODE==OVERCLOCK_800

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


#elif VGA_MODE==400
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

#elif VGA_MODE==320

// 320 240 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps
#define VGA_SKIPLINE
#define VGA_H_PIXELS 320
#define VGA_V_PIXELS 240
#define VGA_V_BLANK 10
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

#elif VGA_MODE==640 // 640x480 default one, use 640 480 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps

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

#else

#error Unknown VGA_MODE !

#endif

#ifdef VGA_SKIPLINE
// line frequency in Hz, should be >30kHz, 31.5 kHz for vesa 640x480
#define VGA_VFREQ (VGA_FPS*2*(VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH))
#else
#define VGA_VFREQ (VGA_FPS*  (VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH))
#endif


#ifndef NO_AUDIO
#define BITBOX_SAMPLERATE 32000 // hsync in fact
#define BITBOX_SNDBUF_LEN 512 // 16ms latency (double buffering is used)
#define BITBOX_SAMPLE_BITDEPTH 8 // 8bit output
#endif