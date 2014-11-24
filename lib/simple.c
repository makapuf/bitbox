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

char vram[SCREEN_H][SCREEN_W];
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
		c = font16_data[(uint8_t) vram[vga_line / 16][i]][vga_line%16];
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
char vram[SCREEN_H][SCREEN_W];

void graph_line() {
	static uint32_t lut_data[4]; // cache couples for faster draws
	
	lut_data[0] = palette[0] <<16 | palette[0];
	lut_data[1] = palette[1] <<16 | palette[0];
	lut_data[2] = palette[0] <<16 | palette[1];
	lut_data[3] = palette[1] <<16 | palette[1];

	uint32_t *dst = (uint32_t *) draw_buffer;
	uint8_t c;

	for (int i=0;i<132;i++) {// column char
		c = font8_data[(uint8_t)vram[vga_line/8][i]][vga_line%8];
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

#elif VGA_SIMPLE_MODE==4 // 4BPP 400x300, base 400x300

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP] = {
	RGB(   0,   0,   0), RGB(   0,   0,0xAA), RGB(   0,0xAA,   0), RGB(   0,0xAA,0xAA),
	RGB(0xAA,   0,   0), RGB(0xAA,   0,0xAA), RGB(0xAA,0x55,   0), RGB(0xAA,0xAA,0xAA),
	RGB(0x55,0x55,0x55), RGB(0x55,0x55,0xFF), RGB(0x55,0xFF,0x55), RGB(0x55,0xFF,0xFF),
	RGB(0xFF,0x55,0x55), RGB(0xFF,0x55,0xFF), RGB(0xFF,0x55,0x55), RGB(0xFF,0xFF,0xFF),
};

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

#elif VGA_SIMPLE_MODE==5 // 8BPP 320x200 - base 320x240  

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP] = { // 256 colors standard VGA palette
	0x0000, 0x0015, 0x02a0, 0x02b5, 0x5400, 0x5415, 0x5540, 0x56b5, 
	0x294a, 0x295f, 0x2bea, 0x2bff, 0x7d4a, 0x7d5f, 0x7fea, 0x7fff, 
	0x0000, 0x0842, 0x1084, 0x14a5, 0x1ce7, 0x2108, 0x294a, 0x318c, 
	0x39ce, 0x4210, 0x4a52, 0x5294, 0x5ad6, 0x6739, 0x739c, 0x7fff, 
	0x001f, 0x201f, 0x3c1f, 0x5c1f, 0x7c1f, 0x7c17, 0x7c0f, 0x7c08, 
	0x7c00, 0x7d00, 0x7de0, 0x7ee0, 0x7fe0, 0x5fe0, 0x3fe0, 0x23e0, 
	0x03e0, 0x03e8, 0x03ef, 0x03f7, 0x03ff, 0x02ff, 0x01ff, 0x011f, 
	0x3dff, 0x4dff, 0x5dff, 0x6dff, 0x7dff, 0x7dfb, 0x7df7, 0x7df3, 
	0x7def, 0x7e6f, 0x7eef, 0x7f6f, 0x7fef, 0x6fef, 0x5fef, 0x4fef, 
	0x3fef, 0x3ff3, 0x3ff7, 0x3ffb, 0x3fff, 0x3f7f, 0x3eff, 0x3e7f, 
	0x5adf, 0x62df, 0x6edf, 0x76df, 0x7edf, 0x7edd, 0x7edb, 0x7ed8, 
	0x7ed6, 0x7f16, 0x7f76, 0x7fb6, 0x7ff6, 0x77f6, 0x6ff6, 0x63f6, 
	0x5bf6, 0x5bf8, 0x5bfb, 0x5bfd, 0x5bff, 0x5bbf, 0x5b7f, 0x5b1f, 
	0x000e, 0x0c0e, 0x1c0e, 0x280e, 0x380e, 0x380a, 0x3807, 0x3803, 
	0x3800, 0x3860, 0x38e0, 0x3940, 0x39c0, 0x29c0, 0x1dc0, 0x0dc0, 
	0x01c0, 0x01c3, 0x01c7, 0x01ca, 0x01ce, 0x014e, 0x00ee, 0x006e, 
	0x1cee, 0x20ee, 0x28ee, 0x30ee, 0x38ee, 0x38ec, 0x38ea, 0x38e8, 
	0x38e7, 0x3907, 0x3947, 0x3987, 0x39c7, 0x31c7, 0x29c7, 0x21c7, 
	0x1dc7, 0x1dc8, 0x1dca, 0x1dcc, 0x1dce, 0x1d8e, 0x1d4e, 0x1d0e, 
	0x294e, 0x2d4e, 0x314e, 0x354e, 0x394e, 0x394d, 0x394c, 0x394b, 
	0x394a, 0x396a, 0x398a, 0x39aa, 0x39ca, 0x35ca, 0x31ca, 0x2dca, 
	0x29ca, 0x29cb, 0x29cc, 0x29cd, 0x29ce, 0x29ae, 0x298e, 0x296e, 
	0x0008, 0x0808, 0x1008, 0x1808, 0x2008, 0x2006, 0x2004, 0x2002, 
	0x2000, 0x2040, 0x2080, 0x20c0, 0x2100, 0x1900, 0x1100, 0x0900, 
	0x0100, 0x0102, 0x0104, 0x0106, 0x0108, 0x00c8, 0x0088, 0x0048, 
	0x1088, 0x1488, 0x1888, 0x1c88, 0x2088, 0x2087, 0x2086, 0x2085, 
	0x2084, 0x20a4, 0x20c4, 0x20e4, 0x2104, 0x1d04, 0x1904, 0x1504, 
	0x1104, 0x1105, 0x1106, 0x1107, 0x1108, 0x10e8, 0x10c8, 0x10a8, 
	0x14a8, 0x18a8, 0x18a8, 0x1ca8, 0x20a8, 0x20a7, 0x20a6, 0x20a6, 
	0x20a5, 0x20c5, 0x20c5, 0x20e5, 0x2105, 0x1d05, 0x1905, 0x1905, 
	0x1505, 0x1506, 0x1506, 0x1507, 0x1508, 0x14e8, 0x14c8, 0x14c8, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

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
}


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
#else 

void print_at(int column, int line, const char *msg)
{
	for(int k = 0; msg[k]; k++)
		vram[line][column + k] = msg[k];
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (int x1, int y1, int x2, int y2 )
{
	for (int i=x1+1;i<x2;i++) 
	{
		vram[y1][i]='\xCD';
		vram[y2][i]='\xCD';
	}
	for (int i=y1+1;i<y2;i++) 
	{
		vram[i][x1]='\xBA';
		vram[i][x2]='\xBA';
	}
	vram[y1][x1] ='\xC9';
	vram[y1][x2] ='\xBB'; 
	vram[y2][x1] ='\xC8'; 
	vram[y2][x2] ='\xBC'; 
}
#endif
