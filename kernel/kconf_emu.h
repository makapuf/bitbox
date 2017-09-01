/* This is bitbox emulated configuration options.
 */
#pragma once
#include <stdint.h>

#ifndef VGA_MODE
#define VGA_MODE 640
#endif 

#ifndef VGA_BPP
#define VGA_BPP 16
#endif 

#define VGA_FPS 60
#define CCM_MEMORY

#define VGA_V_SYNC 16 // simulates 16 lines of vsync

#if VGA_MODE==800 || VGA_MODE==OVERCLOCK_800

// 800 600 non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_H_PIXELS 800
#define VGA_V_PIXELS 600

#elif VGA_MODE==400
// 400x300 based on 800x600 + skipline / non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_SKIPLINE
#define VGA_H_PIXELS 400
#define VGA_V_PIXELS 300

#elif VGA_MODE==320

// 320 240 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps
#define VGA_SKIPLINE
#define VGA_H_PIXELS 320
#define VGA_V_PIXELS 240

#elif VGA_MODE==NONE
// .. nothing here

#else // default 640x480

#define VGA_H_PIXELS 640
#define VGA_V_PIXELS 480
#endif

#ifndef NO_AUDIO

#ifndef BITBOX_SAMPLERATE
#define BITBOX_SAMPLERATE 32000 // hsync in fact
#endif

#ifndef BITBOX_SNDBUF_LEN
#define BITBOX_SNDBUF_LEN 512 // 16ms latency (double buffering is used)
#endif

#ifndef BITBOX_SAMPLE_BITDEPTH
#define BITBOX_SAMPLE_BITDEPTH 8 // 8bit output
#endif

#endif // NO_AUDIO
