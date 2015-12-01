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
#elif VGA_SIMPLE_MODE==4
  #define VGAMODE_400
#elif VGA_SIMPLE_MODE==5
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
#endif 

// --------------------------------------

#define SYSCLK (8000000UL * PLL_N / PLL_P / PLL_M ) // HZ (and 8*n/q/m = 48MHz)

#ifndef VGAMODE_NONE 
#ifdef VGA_SKIPLINE 
// line frequency in Hz, should be >30kHz, 31.5 kHz for vesa 640x480
#define VGA_VFREQ (VGA_FPS*2*(VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH)) 
#else 
#define VGA_VFREQ (VGA_FPS*  (VGA_V_PIXELS+VGA_V_BACKPORCH+VGA_V_SYNC+VGA_V_FRONTPORCH)) 
#endif
#endif 