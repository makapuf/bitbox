/* This is bitbox standard kernel configuration options. A standard dev should not modify it,
 * as this one is used to tune clocks / overclocking, VGA clocks ...
 *
 * to override this file in your project, just include a modified kconf.h in your main compile dir.
 */

// --------------------------------------
#pragma once

#ifdef BOARD_MICRO
#include "kconf_micro.h"
#elif defined(BOARD_BITBOX)
#include "kconf_bitbox.h"
#elif defined(BOARD_PAL)
#include "kconf_pal.h"
#elif defined(EMULATOR)
#include "kconf_emu.h"
#else 
#error no known board
#endif

// --------------------------------------
#ifndef EMULATOR
#define SYSCLK (8000000UL * PLL_N / PLL_P / PLL_M ) // HZ (and 8*n/q/m = 48MHz)
#endif
