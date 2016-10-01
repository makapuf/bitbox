#include "framebuffer.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset


uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP];

// --------------------------------------------------------------

#if FRAMEBUFFER_BPP==1 

const uint16_t initial_palette[] = {0, RGB(0xAA, 0xAA, 0xAA)};

uint32_t cp[1<<(BPP*2)]; // couples palette

void graph_frame()
{
	// load palette into couple palette
	cp[0] = palette[0]<<16|palette[0];
	cp[1] = palette[0]<<16|palette[1];
	cp[2] = palette[1]<<16|palette[0];
	cp[3] = palette[1]<<16|palette[1];
}

void graph_line() {
	if (vga_odd) return;
	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[vga_line*VGA_H_PIXELS/32];
	for (int i=0;i<VGA_H_PIXELS/32;i++) {
		// read 1 word = 32 pixels
		uint32_t w = *src++;
		for (int j=0;j<32;j+=2) // 16 couples of pixels - verify unrolled
			*dst++ = cp[w>>j&3];
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==2 

const uint16_t initial_palette[] = {
	RGB(0,0,0),
	RGB(0x55, 0xff, 0xff),
	RGB(0xff, 0x55, 0x55),
	RGB(0xff, 0xff, 0xff)
};

uint32_t cp[1<<(BPP*2)]; // 4*4=16

void graph_frame()
{
	// reload couple palette
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			cp[j<<2 | i] = palette[j]<<16 | palette[i];
}

void graph_line() {
	if (vga_odd) return;
	// resolution is 640x480, but make letterbox style with 640x400:
	// only draw from y=40 to y=440 (with SCREEN_H=400)
	if (vga_line/2==0 || vga_line/2 == 220) memset(draw_buffer, 0, SCREEN_W*2);
	if (vga_line<40 || vga_line >= 440) return;


	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line-40)*640/16];
	for (int i=0;i<640/16;i++) {
		// read 1 word = 16 pixels
		uint32_t w = *src++;
		// we need to write two pixels at a time into *dst,
		// and increment dst 8 times to get all the pixels from w.
		for (int j=0;j<32;j+=4) // 8 pixels - verify unrolled
			*dst++ = cp[w>>j&15];
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==4 

uint16_t initial_palette[]  = {
	RGB(   0,   0,   0), RGB(   0,   0,0xAA), RGB(   0,0xAA,   0), RGB(   0,0xAA,0xAA),
	RGB(0xAA,   0,   0), RGB(0xAA,   0,0xAA), RGB(0xAA,0x55,   0), RGB(0xAA,0xAA,0xAA),
	RGB(0x55,0x55,0x55), RGB(0x55,0x55,0xFF), RGB(0x55,0xFF,0x55), RGB(0x55,0xFF,0xFF),
	RGB(0xFF,0x55,0x55), RGB(0xFF,0x55,0xFF), RGB(0xFF,0x55,0x55), RGB(0xFF,0xFF,0xFF),
};

void graph_frame() {}
void graph_line() {
	if (vga_odd) return;
	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line)*SCREEN_W/(32/BPP)];

	for (int i=0;i<SCREEN_W/(32/BPP);i++) {
		// read 1 word = 8 pixels
		uint32_t w = *src++;

		for (int j=0;j<32;j+=(32/BPP)) {// 4 couples of pixels - drawn 2 times
			*dst++ = palette[w>>j & 7] | palette[w>>(j+4)&7]<<16;
		}
	}
}

// --------------------------------------------------------------

#elif FRAMEBUFFER_BPP==8 // 400x300 is 120k !

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32] ;
uint16_t palette[1<<BPP]  PALETTE_SECTION;
const uint16_t initial_palette[] = { // 256 colors standard VGA palette
	// XXX replace with micro palette
	0x0000, 0x0015, 0x02a0, 0x02b5, 0x5400, 0x5415, 0x5540, 0x56b5,
	0x294a, 0x295f, 0x2bea, 0x2bff, 0x7d4a, 0x7d5f, 0x7fea, 0x7fff,
};

void graph_frame() {}
void graph_line() {
	if (vga_odd) return;
	// letterbox
	if (vga_line/2==110) memset(draw_buffer, 0, SCREEN_W*2); // at 220 & 221
	if (vga_line<20 || vga_line >= 220) return;

	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line-20)*320/4];

	for (int i=0;i<320/4;i++) {
		// read 1 word = 4 pixels ie 2 couples
		uint32_t w = *src++;
		*dst++ = palette[(w>>8)  & 0xff]<<16 | palette[(w>> 0)&0xff];
		*dst++ = palette[(w>>24) & 0xff]<<16 | palette[(w>>16)&0xff];
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
	int pixel=x+y*SCREEN_W; // number of the pixel
	vram[pixel/(32/BPP)] &= ~ (((1<<BPP)-1)<<(BPP*(pixel%(32/BPP)))); // mask
	vram[pixel/(32/BPP)] |= c<<(BPP*(pixel%(32/BPP))); // value
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