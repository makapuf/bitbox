/* This is bitbox standard kernel configuration options. A standard dev should not modify it,
 * as this one is used to tune clocks / overclocking, VGA clocks ...
 *
 * to override this file in your project, just include a modified kconf.h in your main compile dir.
 */
#include <stdint.h>


// Modes implied by simple Modes

#ifdef VGA_SIMPLE_MODE
#if VGA_SIMPLE_MODE == 1 || VGA_SIMPLE_MODE==2
  #define VGAMODE_800
#elif VGA_SIMPLE_MODE==11
  #define VGAMODE_800_OVERCLOCK
#elif VGA_SIMPLE_MODE==4 || VGA_SIMPLE_MODE==13
  #define VGAMODE_400
#elif VGA_SIMPLE_MODE==5 || VGA_SIMPLE_MODE==8
  #define VGAMODE_320
#else
  #define VGAMODE_640
#endif
#endif

// --------------------------------------

#ifdef BOARD_MICRO
#include "kconf_micro.h"
#elif defined(BOARD_BITBOX)
#include "kconf_bitbox.h"
#elif defined(EMULATOR)
#include "kconf_emu.h"
#elif defined(BOARD_PAL)
#include "kconf_pal.h"
#endif

// --------------------------------------
#ifndef EMULATOR
#define SYSCLK (8000000UL * PLL_N / PLL_P / PLL_M ) // HZ (and 8*n/q/m = 48MHz)
#endif
