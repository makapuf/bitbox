// kconf_pal.h : kernel definitions for PAL (15kHz) modes with standard bitbox

// 168MHz, 15625Hz, 50.08Hz (PAL non-interlace overscan)
// Define PAL_MODE to the desired horizontal resolution, and these macros will
// compute an appropriate mode.
// The "safe" values are listed below.

// 405 usb
#ifndef NO_USB
#define USE_USB_OTG_HS 
#define USE_EMBEDDED_PHY 
#define USE_USB_OTG_FS 
#endif 


// If the game didn't specify anything, let's pick a default mode
#ifndef PAL_MODE

  // Compatible VGA mode : 320x271 (should be 320x240)
  #ifdef VGAMODE_320
    #define PAL_MODE 25 
  #endif

  // PAL-optimized mode 384x271
  #ifdef VGAMODE_360
    #define PAL_MODE 21
  #endif 

  #ifndef PAL_MODE
    #define PAL_MODE 21
  #endif
#endif

#define VGA_PIXELCLOCK (8064/PAL_MODE) // DMA clocks per pixel

// Possible values => Horizontal resolution (8064 / DMA divider)
// (at 168MHz CPU, PLL_N=336)

//  7 and lower => Not possible, maximal DMA speed is 6 cycles per transfer!
//                 (well... may be possible with some hacks, but who need a
//                 resolution that high anyway?)
//  8 =>1008px *
//  9 => 896px
// 10 => 806px (almost Atari ST medres/ CPC mode 2)
// 11 => 733px
// 12 => 672px *
// 13 => 620px
// 14 => 576px *
// 15 => 537px
// 16 => 504px *
// 17 => 474px
// 18 => 448px
// 19 => 424px
// 20 => 403px
// 21 => 384px * (Same as Amstrad CPC mode 1 / Atari ST lowres)
// 22 => 366px
// 23 => 350px
// 24 => 336px *
// 25 => 322px
// 26 => 310px
// 27 => 298px
// 28 => 288px *
// 29 => 278px
// 30 => 268px
// 31 => 260px
// 32 => 252px *
// 33 => 244px
// 34 => 237px
// 35 => 230px
// 36 => 224px
// 37 => 217px
// 38 => 212px
// 39 => 206px
// 40 => 201px
// 41 => 196px
// 42 => 192px * (Same as Amstrad CPC mode 0)
// 43 => 187px
// 44 => 183px
// ...
// 48 => 168px *
// 56 => 144px *
// 64 => 126px *
// 84 =>  96px *
// 96 =>  84px *
// 112=>  72px *
// (multiply one of the above by two to get more with even larger pixels...)
// 672=>  12px (minimum possible value)
//
// Resolutions marked * are "exact", that is, the DMA speed is an integer
// divisor of the horizontal refresh rate. Just so you know, because this doesn't
// seem to have any effect I can think of on anything...

// With 160MHz CPU (PLL_N=320):
// 10 => 1024 (CPC Mode 2/ST medres)
//
// Vertical resolution is fixed at 312 total lines (including sync), of which
// max. 288 are visible (probably a bit less depending on the TV used).

#define PLL_M 8
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7
#define APB_PRESC 2 

// ----
// No changes below this line please.

#define SYSCLK_MHZ 8UL * PLL_N / PLL_M / PLL_P
#define VGA_VFREQ 15625 // line frequency in Hz, fixed for PAL standard

#define VGA_H_PIXELS 48 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_FRONTPORCH 4 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_SYNC 4 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_BACKPORCH 8 * SYSCLK_MHZ / VGA_PIXELCLOCK
// Total of these 4 (*168MHz/PIXELCLOCK) must be 64µS

#define VGA_V_PIXELS 271
#define VGA_V_FRONTPORCH 10
#define VGA_V_SYNC 4
#define VGA_V_BACKPORCH 27
// Total of these 4 must be 312 lines

// ----------------------------------------------------------------------------
// Standard defines, shared with the standard bitbox platform
//
#define HAS_CMM 
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
