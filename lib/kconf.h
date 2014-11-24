/* This is bitbox standard kernel configuration options. A standard dev should not modify it, 
 * as this one is used to tune clocks / overclocking, VGA clocks ... 
 * 
 * to override this file in your project, just include a modified kconf.h in your main compile dir. 
 */

#ifdef VGAMODE_640_OVERCLOCK

// 640 480 VESA on 192 MHz SYSCLK
#define VGA_H_PIXELS 640 
#define VGA_H_FRONTPORCH 16 
#define VGA_H_SYNC 96
#define VGA_H_BACKPORCH 48

#define VGA_V_PIXELS 480
#define VGA_V_FRONTPORCH 10
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 33

#define VGA_FPS 60
#define VGA_PIXELCLOCK 7 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 192L
#define PLL_P 2
#define PLL_Q 8
#define APB_PRESC 2 

#elif defined(VGAMODE_800)

// 800 600 non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_H_PIXELS 800 
#define VGA_H_FRONTPORCH 64 
#define VGA_H_SYNC 128
#define VGA_H_BACKPORCH 50

#define VGA_V_PIXELS 600
#define VGA_V_FRONTPORCH 1
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 14

#define VGA_FPS 56
#define VGA_PIXELCLOCK 5 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 360
#define PLL_P 4
#define PLL_Q 15
#define APB_PRESC 2 

#elif defined(VGAMODE_400)
// 400x300 based on 800x600 + skipline / non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_H_PIXELS 400 
#define VGA_H_FRONTPORCH 32 
#define VGA_H_SYNC 64
#define VGA_H_BACKPORCH 25

#define VGA_SKIPLINE
#define VGA_V_PIXELS 300
#define VGA_V_FRONTPORCH 1
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 7

#define VGA_FPS 56
#define VGA_PIXELCLOCK 10 // DMA clocks per pixel

#define PLL_M 4
#define PLL_N 360
#define PLL_P 4
#define PLL_Q 15
#define APB_PRESC 2 

#elif defined(VGAMODE_320)

// 320 240 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps

#define VGA_H_PIXELS 320 
#define VGA_H_FRONTPORCH 8 
#define VGA_H_SYNC 48
#define VGA_H_BACKPORCH 24

#define VGA_SKIPLINE
#define VGA_V_PIXELS 240
#define VGA_V_FRONTPORCH 2
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 6

#define VGA_FPS 60
#define VGA_PIXELCLOCK 14 // DMA clocks per pixel

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7
#define APB_PRESC 2 

#else // default one, use 
// 640 480 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps
#define VGA_H_PIXELS 640 
#define VGA_H_FRONTPORCH 16 
#define VGA_H_SYNC 64
#define VGA_H_BACKPORCH 80

#define VGA_V_PIXELS 480
#define VGA_V_FRONTPORCH 3
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 13

#define VGA_FPS 60
#define VGA_PIXELCLOCK 7 // DMA clocks per pixel

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7
#define APB_PRESC 2 

#endif


// ----

#define SYSCLK (8000000UL * PLL_N / PLL_M / PLL_P ) // Hz get from MPQ clk ..
#ifdef VGA_SKIPLINE 
#define VGA_VFREQ (VGA_FPS*2*(VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH)) // line frequency in Hz, should be >30kHz, 31.5 kHz for vesa 640x480
#else 
#define VGA_VFREQ (VGA_FPS*(VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH)) // line frequency in Hz, should be >30kHz, 31.5 kHz for vesa 640x480
#endif