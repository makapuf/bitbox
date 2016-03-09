#include "simple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define PALETTE_SECTION CCM_MEMORY

// --------------------------------------------------------------

#if VGA_SIMPLE_MODE==13 // text mode 50x37, font 8x8, base 400

extern const uint8_t font88_data[256][8];
uint8_t font[256][8];
// palette attribute
uint16_t palette[1]={181}; // white, light grey

char vram[SCREEN_H][SCREEN_W];

void graph_frame() {}
void graph_line() {
	// first or second half
	int start_x = vga_odd ? 200 : 0;

	uint32_t *dst = (uint32_t*) &draw_buffer[start_x/2]; // /2 cause draw_buffer is u16
	uint8_t  *src = (uint8_t*) &vram[vga_line/8][start_x/8];

	for (int i=0;i<SCREEN_W/2;i++) {  // draw half-screen
		uint32_t a;
		uint8_t c = font[src[i]][vga_line%8]; // 8 bit font data (to 32 loads ?)

		uint32_t c1=((palette[0]>>0)&0xff)*0x01010101;
		uint32_t c2=((palette[0]>>8)&0xff)*0x01010101;

		// draw a character on this line - 8 pixels = 2 words

		// XXX replace with SEL + MSR

		// select 4 bytes from c1 or c2 according to high bits of word
		a  = c&128 ? c1&0x000000ff : c2&0x000000ff;
		a |= c&64  ? c1&0x0000ff00 : c2&0x0000ff00;
		a |= c&32  ? c1&0x00ff0000 : c2&0x00ff0000;
		a |= c&16 ? c1&0xff000000 : c2&0xff000000;
		*dst++ = a;

		// select 4 bytes from c1 or c2 according to high bits of word
		a  = c&8 ? c1&0x000000ff : c2&0x000000ff;
		a |= c&4 ? c1&0x0000ff00 : c2&0x0000ff00;
		a |= c&2 ? c1&0x00ff0000 : c2&0x00ff0000;
		a |= c&1 ? c1&0xff000000 : c2&0xff000000;
		*dst++ = a;
	}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==4  // 4BPP 400x300

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint8_t palette[1<<BPP]  PALETTE_SECTION;
uint8_t initial_palette[]  = {
	RGB8(   0,   0,   0), RGB8(   0,   0,0xAA), RGB8(   0,0xAA,   0), RGB8(   0,0xAA,0xAA),
	RGB8(0xAA,   0,   0), RGB8(0xAA,   0,0xAA), RGB8(0xAA,0x55,   0), RGB8(0xAA,0xAA,0xAA),
	RGB8(0x55,0x55,0x55), RGB8(0x55,0x55,0xFF), RGB8(0x55,0xFF,0x55), RGB8(0x55,0xFF,0xFF),
	RGB8(0xFF,0x55,0x55), RGB8(0xFF,0x55,0xFF), RGB8(0xFF,0x55,0x55), RGB8(0xFF,0xFF,0xFF),
};

void graph_frame() {}
void graph_line() {
	if (vga_odd) return;
	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line)*SCREEN_W/(32/BPP)];

	for (int i=0;i<SCREEN_W/(32/BPP);i++) {
		uint32_t w = *src++;
		// read 1 word = 8 pixels
		uint32_t d;
		// write first word
		d  = palette[w>>0 &7]<<0;
		d |= palette[w>>4 &7]<<8;
		d |= palette[w>>8 &7]<<16;
		d |= palette[w>>12&7]<<24;
		*dst++ = d;
		// second word
		d  = palette[w>>16&7]<<0;
		d |= palette[w>>20&7]<<8;
		d |= palette[w>>24&7]<<16;
		d |= palette[w>>28&7]<<24;
		*dst++ = d;
	}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==5 // 8BPP 320x200 - base 320x240

#define NOPALETTE
uint32_t vram[SCREEN_W*SCREEN_H*BPP/32]; // easy, no palette

void graph_frame() {}
void graph_line() {
	if (vga_odd) return;
	// letterbox
	if (vga_line/2==110)
		memset(draw_buffer, 0, SCREEN_W*2); // at 220 & 221
	if (vga_line<20 || vga_line >= 220) return;

	memcpy(draw_buffer,&vram[(vga_line-20)*320/4],320);
}


// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==8 // 4BPP 320x200 (base 320x240)


uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint8_t palette[1<<BPP]  PALETTE_SECTION;
uint8_t initial_palette[]  = {
	RGB8(   0,   0,   0), RGB8(   0,   0,0xAA), RGB8(   0,0xAA,   0), RGB8(   0,0xAA,0xAA),
	RGB8(0xAA,   0,   0), RGB8(0xAA,   0,0xAA), RGB8(0xAA,0x55,   0), RGB8(0xAA,0xAA,0xAA),
	RGB8(0x55,0x55,0x55), RGB8(0x55,0x55,0xFF), RGB8(0x55,0xFF,0x55), RGB8(0x55,0xFF,0xFF),
	RGB8(0xFF,0x55,0x55), RGB8(0xFF,0x55,0xFF), RGB8(0xFF,0x55,0x55), RGB8(0xFF,0xFF,0xFF),
};

void graph_frame() {}
void graph_line() {
	if (vga_odd) return;
	// letterbox
	if (vga_line/2==110) memset(draw_buffer, 0, SCREEN_W*2); // at 220 & 221 to empty both line buffers
	if (vga_line<20 || vga_line >= 220) return;

	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[(vga_line-20)*SCREEN_W/(32/BPP)];

	for (int i=0;i<SCREEN_W/(32/BPP);i++) {
		uint32_t w = *src++;
		// read 1 word = 8 pixels
		uint32_t d;
		// write first word
		d  = palette[w>>0 &7]<<0;
		d |= palette[w>>4 &7]<<8;
		d |= palette[w>>8 &7]<<16;
		d |= palette[w>>12&7]<<24;
		*dst++ = d;
		// second word
		d  = palette[w>>16&7]<<0;
		d |= palette[w>>20&7]<<8;
		d |= palette[w>>24&7]<<16;
		d |= palette[w>>28&7]<<24;
		*dst++ = d;
	}
}

// --------------------------------------------------------------
#else

#error This mode is not available on bitbox micro

#endif

// --------------------------------------------------------------
// utilities


#ifdef BPP // this is a graphical mode

void clear()
{
   memset(vram, 0, sizeof(vram));
   #ifndef NOPALETTE
   memcpy(palette,initial_palette,sizeof(initial_palette));
   #endif
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

#else

#ifdef COLOR_TEXT
uint8_t text_color; // current attribute value of the drawn text
#endif

void clear()
{
   memset(vram, 0, sizeof(vram));
   #if VGA_SIMPLE_MODE==0
   memcpy(font16_data_cached, font16_data, sizeof(font16_data_cached));
   #elif VGA_SIMPLE_MODE==1
   memcpy(font8_data_cached, font8_data, sizeof(font8_data_cached));
   #elif VGA_SIMPLE_MODE==10
   memcpy(font16_data_cached, font16_data, sizeof(font16_data_cached));
   memset(vram_attr, 0, sizeof(vram_attr));
   #elif VGA_SIMPLE_MODE==11
   memcpy(font8_data_cached, font8_data, sizeof(font8_data_cached));
   memset(vram_attr, 0, sizeof(vram_attr));
   #elif VGA_SIMPLE_MODE==12
   memcpy(font88_data_cached, font88_data, sizeof(font88_data_cached));
   memset(vram_attr, 0, sizeof(vram_attr));
   #elif VGA_SIMPLE_MODE==13
   memcpy(font,font88_data, sizeof(font));
   #endif
}

void print_at(int column, int line, const char *msg)
{
	for(int k = 0; msg[k] && (k+column+line*SCREEN_W<SCREEN_H*SCREEN_W); k++) {
		vram[line][column + k] = msg[k];
		#ifdef COLOR_TEXT
		vram_attr[line][column + k] = text_color;
		#endif
  	}
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (int x1, int y1, int x2, int y2)
{
	for (int i=x1+1;i<x2;i++)
	{
		vram[y2][i]=vram[y1][i]='\xCD';
		#ifdef COLOR_TEXT
	    vram_attr[y1][i]=vram_attr[y2][i]=text_color;
		#endif

	}
	for (int i=y1+1;i<y2;i++)
	{
		vram[i][x1]=vram[i][x2]='\xBA';
		#ifdef COLOR_TEXT
    	vram_attr[i][x1]=vram_attr[i][x2]=text_color;
		#endif
	}

	vram[y1][x1] ='\xC9';
	vram[y1][x2] ='\xBB';
	vram[y2][x1] ='\xC8';
	vram[y2][x2] ='\xBC';
	#ifdef COLOR_TEXT
	vram_attr[y1][x1] =vram_attr[y1][x2]=vram_attr[y2][x1]=vram_attr[y2][x2]=text_color;
	#endif
}
#endif
