#include "framebuffer.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset


uint32_t vram[VGA_H_PIXELS*VGA_V_PIXELS*FRAMEBUFFER_BPP/32];
pixel_t palette[1<<FRAMEBUFFER_BPP];

#define PPW (32/FRAMEBUFFER_BPP) // source pixels per word

#if VGA_BPP==8
typedef uint16_t couple_t;
#else
typedef uint32_t couple_t;
#endif
// --------------------------------------------------------------

#if FRAMEBUFFER_BPP==1 

const pixel_t initial_palette[] = {0, RGB(0xAA, 0xAA, 0xAA)};

couple_t cp[1<<(FRAMEBUFFER_BPP*2)]; // 4 couples palette

void graph_vsync()
{
	if (vga_odd || vga_line != VGA_V_PIXELS+2) return;
	// refresh couple palette
	cp[0] = palette[0]<<VGA_BPP|palette[0];
	cp[1] = palette[0]<<VGA_BPP|palette[1];
	cp[2] = palette[1]<<VGA_BPP|palette[0];
	cp[3] = palette[1]<<VGA_BPP|palette[1];
}

void graph_line() {
	if (vga_odd) return;

	// read & write words
	uint32_t *src=&vram[vga_line*VGA_H_PIXELS/32]; 
	uint32_t *dst=(uint32_t*)draw_buffer;

	for (int i=0;i<VGA_H_PIXELS/32;i++) { // loop over input words
		uint32_t w = *src++;  // read 1 word = 32 pixels
		#if VGA_BPP==8
		for (int j=0;j<32;j+=4) // 8 quads of pixels
			*dst++ = cp[w>>j&3] | cp[w>>(j+2)&3]<<16;
		#else 
		for (int j=0;j<32;j+=2) // 16 couples of pixels 
			*dst++ = cp[w>>j&3];
		#endif 
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==2 

const pixel_t initial_palette[] = {
	RGB(0,0,0),
	RGB(0x55, 0xff, 0xff),
	RGB(0xff, 0x55, 0x55),
	RGB(0xff, 0xff, 0xff)
};

couple_t cp[1<<(FRAMEBUFFER_BPP*2)]; // 4*4=16

void graph_vsync()
{
	if (vga_odd || vga_line != VGA_V_PIXELS+2) return;
	// refresh couple palette
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			cp[j<<2 | i] = palette[j]<<VGA_BPP | palette[i];
	}

	void graph_line() {
		if (vga_odd) return;

		uint32_t *src=&vram[vga_line*VGA_H_PIXELS/16];
		uint32_t *dst=(uint32_t*)draw_buffer;

		for (int i=0;i<VGA_H_PIXELS/16;i++) {
		uint32_t w = *src++; // read 1 word = 16 pixels
		
		#if VGA_BPP==8
		for (int j=0;j<32;j+=8) // 4 quads
			*dst++ = cp[w>>j&15] | cp[w>>(j+4)&15]<<16;
		#else 
		for (int j=0;j<32;j+=4) // 8 couples
			*dst++ = cp[w>>j&15];
		#endif
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==4 

pixel_t initial_palette[]  = {
	RGB(   0,   0,   0), RGB(   0,   0,0xAA), RGB(   0,0xAA,   0), RGB(   0,0xAA,0xAA),
	RGB(0xAA,   0,   0), RGB(0xAA,   0,0xAA), RGB(0xAA,0x55,   0), RGB(0xAA,0xAA,0xAA),
	RGB(0x55,0x55,0x55), RGB(0x55,0x55,0xFF), RGB(0x55,0xFF,0x55), RGB(0x55,0xFF,0xFF),
	RGB(0xFF,0x55,0x55), RGB(0xFF,0x55,0xFF), RGB(0xFF,0x55,0x55), RGB(0xFF,0xFF,0xFF),
};

void graph_line() {
	if (vga_odd) return;
	uint32_t *src=&vram[(vga_line)*VGA_H_PIXELS/8];
	uint32_t *dst=(uint32_t*)draw_buffer;

	for (int i=0;i<VGA_H_PIXELS/8;i++) {
		uint32_t w = *src++; // read 1 word = 8 pixels

		#if VGA_BPP==8
		for (int j=0;j<32;j+=16) { // 2 quads of pixels 
			uint32_t q;
			q  = palette[w>>(j+12)&7]<<24;
			q |= palette[w>>(j+ 8)&7]<<16;
			q |= palette[w>>(j+ 4)&7]<<8;
			q |= palette[w>>j & 7];
			*dst++ = q;
		}	
		#else
		for (int j=0;j<32;j+=8) { // 4 couples of pixels 
			*dst++ = palette[w>>j & 7] | palette[w>>(j+4)&7]<<16;
		}
		#endif
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==8 // 400x300 is 120k in that mode ! 640x480 is not feasible
// on micro, 320x240 eats 76k : almost all ram (also, no palette !) 

const uint16_t initial_palette[] = { // 256 colors standard VGA palette
	// XXX replace with micro palette
	0x0000, 0x0015, 0x02a0, 0x02b5, 0x5400, 0x5415, 0x5540, 0x56b5,
	0x294a, 0x295f, 0x2bea, 0x2bff, 0x7d4a, 0x7d5f, 0x7fea, 0x7fff,
};

void graph_line() {
	if (vga_odd) return;
	// letterbox
	/*
	if (vga_line/2==110) memset(draw_buffer, 0, SCREEN_W*2); // at 220 & 221
	if (vga_line<20 || vga_line >= 220) return;
	*/

	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[vga_line*VGA_H_PIXELS/4];

	for (int i=0;i<VGA_H_PIXELS/4;i++) {
		// read 1 word = 4 pixels ie 2 couples
		uint32_t w = *src++;
		#if VGA_BPP==8
		// no palette, just copy
		*dst++ = w;
		#else 
		*dst++ = palette[(w>>8)  & 0xff]<<16 | palette[(w>> 0)&0xff];
		*dst++ = palette[(w>>24) & 0xff]<<16 | palette[(w>>16)&0xff];
		#endif 
	}
}

#endif

// --------------------------------------------------------------
// utilities


void clear()
{
	memset(vram, 0, sizeof(vram));
	memcpy(palette,initial_palette,sizeof(initial_palette));
}

void draw_pixel(int x,int y,int c)
{
	int pixel=x+y*VGA_H_PIXELS; // number of the pixel
	vram[pixel/PPW] &= ~ (((1<<FRAMEBUFFER_BPP)-1)<<(FRAMEBUFFER_BPP*(pixel%PPW))); // mask
	vram[pixel/PPW] |= c<<(FRAMEBUFFER_BPP*(pixel%PPW)); // value
	// if e.g. BPP == 2 (640x400 mode)
	// you fit 32/BPP = 16 pixels in one 32bit integer
}

void draw_line(int x0, int y0, int x1, int y1, int c) {
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		draw_pixel(x0,y0,c);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

// draw text at 8x8 ?