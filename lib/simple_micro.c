#include "simple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define PALETTE_SECTION CCM_MEMORY

// --------------------------------------------------------------

#if VGA_SIMPLE_MODE==13 // text mode 50x37, font 8x8, base 400
extern const uint8_t font88_data[256][8];
uint8_t font88_data_cached[256][8] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
char vram_attr[SCREEN_H][SCREEN_W];
uint16_t palette[256]; // BG<<8 | FG couples 

void graph_frame() {}
void graph_line8() 
{
	uint16_t lut_data[4]; // cache couples for faster draws

	uint32_t *dst = (uint32_t*)draw_buffer;
	if (vga_odd) 
		dst += VGA_H_PIXELS/8;
	uint8_t prev_attr = 0xff; // what if it's just that ?
	
	for (int i=0;i<SCREEN_W/2;i++) { // column char
		// draw a character on this line
		uint8_t p = font88_data_cached[(uint8_t) vram[vga_line / 8][vga_odd ? i+SCREEN_W/2:i]][vga_line%8];
		uint8_t attr = vram_attr[vga_line/8][vga_odd ? i+SCREEN_W/2 : i];
		if (attr != prev_attr) {
			uint16_t c = palette[attr];

			lut_data[0] = (c&0xff)*0x101; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<8 | c>>8); // BA
			lut_data[3] = (c>>8)*0x101; // BB

			prev_attr = attr;
		}

		// draw a character on this line
		*dst++ = lut_data[(p>>6) & 0x3] | lut_data[(p>>4) & 0x3]<<16;
		*dst++ = lut_data[(p>>2) & 0x3] | lut_data[(p>>0) & 0x3]<<16;
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
void graph_line8() {
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
void graph_line8() {
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
void graph_line8() {
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
   memset(vram, ' ', sizeof(vram));
   #if VGA_SIMPLE_MODE==13
   memcpy(font88_data_cached, font88_data, sizeof(font88_data_cached));
   memset(vram_attr, 0, sizeof(vram_attr));
   palette[0]=0x00DE;
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
