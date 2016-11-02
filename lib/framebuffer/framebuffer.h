/* A simple framebuffer based engine.
 * You can write to VRAM and the display will be done from this VRAM.
 *
 * - define FRAMEBUFFER_BPP=1,2,4(default) or 8 to define the framebuffer depth.
 * - resolution will be given by the mode you're using.
 * - VRAM size = x*y*BPP/8 (ex 400x300 @ 4bpp is 60kB)
 *
 */ 

#include <stdint.h>

// TODO define window for overscan to lower RAM usage ?

#ifndef FRAMEBUFFER_BPP
#define FRAMEBUFFER_BPP 4
#endif 

void clear(); // always clear before first use

extern uint32_t vram[]; // Pixel data, here defined as words.
extern pixel_t palette[1<<FRAMEBUFFER_BPP];

void draw_pixel(int x, int y, int c);
void draw_line(int x0, int y0, int x1, int y1, int c);
