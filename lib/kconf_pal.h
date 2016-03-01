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

/* Try to match the VGA simple modes as much as possible. There is often no
 * exact match possible, so we add or remove a few pixels.
 * No matter what we do, the vertical resolution is always 271 lines. */
#ifdef VGA_SIMPLE_MODE
#if VGA_SIMPLE_MODE == 1 || VGA_SIMPLE_MODE==2
  #define PAL_MODE 806
#elif VGA_SIMPLE_MODE==11 
  #define PAL_MODE 733
#elif VGA_SIMPLE_MODE==4
  #define PAL_MODE 403
#elif VGA_SIMPLE_MODE==5 || VGA_SIMPLE_MODE==8
  #define PAL_MODE 310
#else 
  #define PAL_MODE 620
#endif 
#endif

// If the game didn't specify anything, let's pick a default mode
#ifndef PAL_MODE

  // Compatible VGA mode : 320x271 (should be 320x240)
  #ifdef VGAMODE_320
    #define PAL_MODE 322
  #endif

  // PAL-optimized mode 384x271
  #ifdef VGAMODE_384
    #define PAL_MODE 384
  #endif

  // Really don't care? Let's pick a default one then.
  #ifndef PAL_MODE
    #define PAL_MODE 384
  #endif
#endif

#define VGA_PIXELCLOCK (8064/PAL_MODE) // DMA clocks per pixel

// Possible values for PAL_MODE (in pixels):
// (at 168MHz CPU, PLL_N=336)

// 1008* (maximal resolution)
//  896
//  806 (slightly lower than Atari ST medres / CPC mode 2)
//  733, 672*, 620, 576*, 537, 504*, 474, 448, 424, 403
//  384* (same as Amstrad CPC Mode 1 / Atari ST lowres)
//  366, 350, 336*, 322, 310, 298, 288*, 278, 268, 260, 252*, 244, 237, 230,
//  224, 217, 212, 206, 201, 196,
//  192* (same as Amstrad CPC mode 0)
//  187, 183, 179, 175, 171, 168*, 164, 161, 158, 155, 152, 149, 146, 144*,
//  141, 139, 136, 134, 132, 130, 128, 126*, 124, 122, 120, 118, 116, 115,
//  ... 96*, ..., 84*, ..., 72*, ...,
//  12 (lowest possible resolution)
//
// Resolutions marked * are "exact", that is, the DMA speed is an integer
// divisor of the horizontal refresh rate. Just so you know, because this doesn't
// seem to have any effect I can think of on anything...

// With 160MHz CPU (PLL_N=320):
// 10 => 1024 (CPC Mode 2/ST medres)
//
// Vertical resolution is fixed at 312 total lines (including sync), of which
// max. 288 are visible (probably a bit less depending on the TV used). We add
// a safety margin to that and make 271 lines part of the framebuffer. This can
// be further reduced, as long as the front and backporch are adjusted to keep
// the total 312 lines. For example it can be set to 200 or 240.

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
