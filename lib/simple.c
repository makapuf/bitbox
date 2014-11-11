#include "simple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

// TODO : text attr (ie 16 colors BG / FG)
// default palettes

void graph_frame() {}
// --------------------------------------------------------------
#if VGA_SIMPLE_MODE==0 // text mode 80x25 

extern const uint8_t font16_data[256][16];

char vram[SCREEN_W*SCREEN_H];
uint16_t palette[2]={0,0x56b5}; 

void graph_line() {
	uint32_t lut_data[4]; // cache couples for faster draws
	
	lut_data[0] = palette[0] <<16 | palette[0];
	lut_data[1] = palette[1] <<16 | palette[0];
	lut_data[2] = palette[0] <<16 | palette[1];
	lut_data[3] = palette[1] <<16 | palette[1];

	uint32_t *dst = (uint32_t *) draw_buffer;
	uint8_t c;

	for (int i=0;i<80;i++) {// column char
		c = font16_data[(uint8_t) vram[(vga_line / 16)*80+i]][vga_line%16];
		// draw a character on this line
		*dst++ = lut_data[(c>>6) & 0x3];
		*dst++ = lut_data[(c>>4) & 0x3];
		*dst++ = lut_data[(c>>2) & 0x3];
		*dst++ = lut_data[(c>>0) & 0x3];
	}
}

// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==1 // mini text mode 132x75, chars 8x6, base 800

extern const uint8_t font8_data[256][8];

uint16_t palette[2]={0,0x56b5}; 
char vram[SCREEN_W*SCREEN_H];

void graph_line() {
	static uint32_t lut_data[4]; // cache couples for faster draws
	
	lut_data[0] = palette[0] <<16 | palette[0];
	lut_data[1] = palette[1] <<16 | palette[0];
	lut_data[2] = palette[0] <<16 | palette[1];
	lut_data[3] = palette[1] <<16 | palette[1];

	uint32_t *dst = (uint32_t *) draw_buffer;
	uint8_t c;

	for (int i=0;i<132;i++) {// column char
		c = font8_data[(uint8_t)vram[(vga_line/8)*132+i]][vga_line%8];
		// draw a character on this line - 6 pixels is 3 couples ie 3 u32
		*dst++ = lut_data[(c>>4) & 0x3];
		*dst++ = lut_data[(c>>2) & 0x3];
		*dst++ = lut_data[(c>>0) & 0x3];
	}
}

// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==2 // 800x600 1bpp - base 800

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[2] = {0, RGB(0xAA, 0xAA, 0xAA)};

void graph_line() {
	unit32_t *dst=(uint32_t)draw_buffer;
	uint32_t *src=&vram[vga_line*800/32];
	for (int i=0;i<800/32;i++) {
		// read 1 word = 32 pixels
		uint32_t w = *src++;
		// test zero / one (faster)

		uint32_t c0 = palette[0]<<16|palette[0];
		uint32_t c1 = palette[0]<<16|palette[1];
		uint32_t c2 = palette[1]<<16|palette[0];
		uint32_t c3 = palette[1]<<16|palette[1];

		for (int j=0;j<32;j+=2) // 16 couples of pixels - verify unrolled
			switch (w>>j&3) {
				case 0 : *dst++ = c0; break;
				case 1 : *dst++ = c1; break;
				case 2 : *dst++ = c2; break;
				case 3 : *dst++ = c3; break;
			}
	}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==3 // 640x400 2BPP + bandes noires (one drawn at 620!)

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP] = {RGB(0,0,0), RGB(0x55, 0xff, 0xff), RGB(0xff, 0x55, 0x55), RGB(0xff, 0xff, 0xff)};

void graph_line() {
	// letterbox
	if (vga_line==620) fast_fill(0,640,RGB(0,0,0));
	if (vga_line<20 || vga_line >= 620) return;
	
	unit32_t *dst=(uint32_t)draw_buffer;
	uint32_t *src=&vram[(vga_line-20)*640/16];
	for (int i=0;i<640/16;i++) {
		// read 1 word = 16 pixels
		uint32_t w = *src++;
		uint32_t c0 = palette[0];
		uint32_t c1 = palette[1];
		uint32_t c2 = palette[2];
		uint32_t c3 = palette[3];

		for (int j=0;j<32;j+=2) // 16 couples of pixels - verify unrolled
			switch (w>>j&3) {
				case 0x0 : *dst++ = c0<<16 | c0; break;
				case 0x1 : *dst++ = c0<<16 | c1; break;
				case 0x2 : *dst++ = c0<<16 | c2; break;
				case 0x3 : *dst++ = c0<<16 | c3; break;
				case 0x4 : *dst++ = c1<<16 | c0; break;
				case 0x5 : *dst++ = c1<<16 | c1; break;
				case 0x6 : *dst++ = c1<<16 | c2; break;
				case 0x7 : *dst++ = c1<<16 | c3; break;
				case 0x8 : *dst++ = c2<<16 | c0; break;
				case 0x9 : *dst++ = c2<<16 | c1; break;
				case 0xa : *dst++ = c2<<16 | c2; break;
				case 0xb : *dst++ = c2<<16 | c3; break;
				case 0xc : *dst++ = c3<<16 | c0; break;
				case 0xd : *dst++ = c3<<16 | c1; break;
				case 0xe : *dst++ = c3<<16 | c2; break;
				case 0xf : *dst++ = c3<<16 | c3; break;
			}
		}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==4 // 4BPP 400x300 64k base 800x600

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP] = {
	RGB(   0,   0,   0), RGB(   0,   0,0xAA), RGB(   0,0xAA,   0), RGB(   0,0xAA,0xAA),
	RGB(0xAA,   0,   0), RGB(0xAA,   0,0xAA), RGB(0xAA,0x55,   0), RGB(0xAA,0xAA,0xAA),
	RGB(0x55,0x55,0x55), RGB(0x55,0x55,0xFF), RGB(0x55,0xFF,0x55), RGB(0x55,0xFF,0xFF),
	RGB(0xFF,0x55,0x55), RGB(0xFF,0x55,0xFF), RGB(0xFF,0x55,0x55), RGB(0xFF,0xFF,0xFF),
};

void graph_line() {
	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line/2)*SCREEN_W/(32/BPP)];

	for (int i=0;i<SCREEN_W/(32/BPP);i++) {
		// read 1 word = 8 pixels
		uint32_t w = *src++;	

		for (int j=0;j<32;j+=(32/BPP)) {// 4 couples of pixels - drawn 2 times 
			*dst++ = palette[w>>j & 7]*0x10001 ;
			*dst++ = palette[w>>(j+4)&7]*0x10001;
		}
	}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==5 // 8BPP 320x200 64k - base 640x480 - one 

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP];

void graph_line() {
	// letterbox
	if (vga_line==620) fast_fill(0,640,RGB(0,0,0));
	if (vga_line<20 || vga_line >= 620) return;

	uint32_t *dst=(uint32_t)draw_buffer;
	uint32_t *src=&vram[(vga_line/2)*320/4];

	for (int i=0;i<320/4;i++) {
		// read 1 word = 4 pixels ie 2 couples
		uint32_t w = *src++;	
		*dst++ = palette[w>>24 & 0xff]<<16 | palette[w>>16&0xff];
		*dst++ = palette[w>>08 & 0xff]<<16 | palette[w>>00&0xff];
	}
}

#endif

// --------------------------------------------------------------

#ifdef BPP // this is a graphical mode
void draw_pixel(int x,int y,int c)
{	
	int pixel=x+y*SCREEN_W; // number of the pixel
	vram[pixel/(32/BPP)] &= ~ (((1<<BPP)-1)<<(BPP*(pixel%(32/BPP)))); // mask
	vram[pixel/(32/BPP)] |= c<<(BPP*(pixel%(32/BPP))); // value
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
void clear() 
{
   memset(vram, 0, 400*300/2);
}

#endif

// SOUND ---------------------------------------
void game_snd_buffer(uint16_t *buffer, int len) {}