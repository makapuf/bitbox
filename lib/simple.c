#include "simple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

// make sure to use CCM_MEMORY instead of .ccm for Mac based builds...
#define PALETTE_SECTION //__attribute__ ((section (".ccm")))


// --------------------------------------------------------------
#if VGA_SIMPLE_MODE==0 // text mode 80x25

extern const uint8_t font16_data[256][16];
uint8_t font16_data_cached[256][16] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
uint16_t palette[2]={0,0x56b5};

void graph_frame() {}
void graph_line() {
	uint32_t lut_data[4]; // cache couples for faster draws

	lut_data[0] = palette[0] <<16 | palette[0];
	lut_data[1] = palette[1] <<16 | palette[0];
	lut_data[2] = palette[0] <<16 | palette[1];
	lut_data[3] = palette[1] <<16 | palette[1];

	uint32_t *dst = (uint32_t*) draw_buffer;
	uint8_t c;

	for (int i=0;i<80;i++) {// column char
		c = font16_data_cached[(uint8_t) vram[vga_line / 16][i]][vga_line%16];
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
uint8_t font8_data_cached[256][8] CCM_MEMORY;

uint16_t palette[2]={0,0x56b5};
char vram[SCREEN_H][SCREEN_W];

void graph_frame() {}
void graph_line() {
	static uint32_t lut_data[4]; // cache couples for faster draws

	lut_data[0] = palette[0] <<16 | palette[0];
	lut_data[1] = palette[1] <<16 | palette[0];
	lut_data[2] = palette[0] <<16 | palette[1];
	lut_data[3] = palette[1] <<16 | palette[1];

	uint32_t *dst = (uint32_t*) draw_buffer;
	uint8_t c;

	for (int i=0;i<132;i++) {// column char
		c = font8_data_cached[(uint8_t)vram[vga_line/8][i]][vga_line%8];
		// draw a character on this line - 6 pixels is 3 couples ie 3 u32
		*dst++ = lut_data[(c>>4) & 0x3];
		*dst++ = lut_data[(c>>2) & 0x3];
		*dst++ = lut_data[(c>>0) & 0x3];
	}
}


// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==2 // 800x600 1bpp - base 800


uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[2]  PALETTE_SECTION;
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
	uint32_t *dst=(uint32_t*)draw_buffer;
	uint32_t *src=&vram[vga_line*800/32];
	for (int i=0;i<800/32;i++) {
		// read 1 word = 32 pixels
		uint32_t w = *src++;
		for (int j=0;j<32;j+=2) // 16 couples of pixels - verify unrolled
			*dst++ = cp[w>>j&3];
	}
}

// --------------------------------------------------------------

#elif VGA_SIMPLE_MODE==3 // 640x400 2BPP + bandes noires

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP]  PALETTE_SECTION;
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

#elif VGA_SIMPLE_MODE==4  // 4BPP 400x300, base 800x600

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP]  PALETTE_SECTION;
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

#elif VGA_SIMPLE_MODE==5 // 8BPP 320x200 - base 320x240

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



#elif VGA_SIMPLE_MODE==8 // 4BPP 320x200 (base 320x240)

uint32_t vram[SCREEN_W*SCREEN_H*BPP/32];
uint16_t palette[1<<BPP]  PALETTE_SECTION;
uint16_t initial_palette[]  = {
	RGB(   0,   0,   0), RGB(   0,   0,0xAA), RGB(   0,0xAA,   0), RGB(   0,0xAA,0xAA),
	RGB(0xAA,   0,   0), RGB(0xAA,   0,0xAA), RGB(0xAA,0x55,   0), RGB(0xAA,0xAA,0xAA),
	RGB(0x55,0x55,0x55), RGB(0x55,0x55,0xFF), RGB(0x55,0xFF,0x55), RGB(0x55,0xFF,0xFF),
	RGB(0xFF,0x55,0x55), RGB(0xFF,0x55,0xFF), RGB(0xFF,0x55,0x55), RGB(0xFF,0xFF,0xFF),
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
		// read 1 word = 8 pixels
		uint32_t w = *src++;

		for (int j=0;j<32;j+=(32/BPP)) {// 4 couples of pixels - drawn 2 times
			*dst++ = palette[w>>j & 15] | palette[w>>(j+4)&15]<<16;
		}
	}
}



// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==10 // text mode 80x30 with colors

extern const uint8_t font16_data[256][16];
uint8_t font16_data_cached[256][16] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
char vram_attr[SCREEN_H][SCREEN_W];
uint32_t palette[256]; // BG<<16 | FG couples (default values : 16c BG, 16c FG)

void graph_frame() {}
void graph_line() {
	uint32_t lut_data[4]; // cache couples for faster draws

	uint32_t *dst = (uint32_t*) draw_buffer;
	uint8_t prev_attr = 0xff; // what if it's just that ?

	for (int i=0;i<SCREEN_W;i++) { // column char
		// draw a character on this line
		uint8_t p = font16_data_cached[(uint8_t) vram[vga_line / 16][i]][vga_line%16];
		uint8_t attr = vram_attr[vga_line/16][i];
		if (attr != prev_attr) {
			uint32_t c = palette[attr];

			lut_data[0] = (c&0xffff)*0x10001; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<16 | c>>16); // BA
			lut_data[3] = (c>>16)*0x10001; // BB

			prev_attr = attr;
		}

		// draw a character on this line
		*dst++ = lut_data[(p>>6) & 0x3];
		*dst++ = lut_data[(p>>4) & 0x3];
		*dst++ = lut_data[(p>>2) & 0x3];
		*dst++ = lut_data[(p>>0) & 0x3];
	}
}

// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==11 // text mode 132x75 with colors

extern const uint8_t font8_data[256][8];
uint8_t font8_data_cached[256][8] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
char vram_attr[SCREEN_H][SCREEN_W];
uint32_t palette[256]; // BG<<16 | FG couples (default values : 16c BG, 16c FG)

void graph_frame() {}
void graph_line( void )
{
	uint32_t lut_data[4]; // cache couples for faster draws

	uint32_t *dst = (uint32_t*) draw_buffer;
	uint8_t prev_attr = 0xff; // what if it's just that ?

	for (int i=0;i<SCREEN_W/2;i++) { // column char
		// draw a character on this line
		uint8_t attr = vram_attr[vga_line/8][i*2];
		if (attr != prev_attr) {
			uint32_t c = palette[attr];

			lut_data[0] = (c&0xffff)*0x10001; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<16 | c>>16); // BA
			lut_data[3] = (c>>16)*0x10001; // BB

			prev_attr = attr;
		}

		uint8_t p = font8_data_cached[(uint8_t) vram[vga_line / 8][i*2]][vga_line%8];
		// draw a character on this line
		*dst++ = lut_data[(p>>4) & 0x3];
		*dst++ = lut_data[(p>>2) & 0x3];
		*dst++ = lut_data[(p>>0) & 0x3];


		attr = vram_attr[vga_line/8][i*2+1];
		if (attr != prev_attr) {
			uint32_t c = palette[attr];

			lut_data[0] = (c&0xffff)*0x10001; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<16 | c>>16); // BA
			lut_data[3] = (c>>16)*0x10001; // BB

			prev_attr = attr;
		}

		p = font8_data_cached[(uint8_t) vram[vga_line / 8][i*2+1]][vga_line%8];
		// draw a character on this line
		*dst++ = lut_data[(p>>4) & 0x3];
		*dst++ = lut_data[(p>>2) & 0x3];
		*dst++ = lut_data[(p>>0) & 0x3];
	}
}


// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==12 // text mode 80x60 with colors (8x8)

extern const uint8_t font88_data[256][8];
uint8_t font88_data_cached[256][8] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
char vram_attr[SCREEN_H][SCREEN_W];
uint32_t palette[256]; // BG<<16 | FG couples (default values : 16c BG, 16c FG)

void graph_frame() {}
void graph_line() {
	uint32_t lut_data[4]; // cache couples for faster draws

	uint32_t *dst = (uint32_t*) draw_buffer;
	uint8_t prev_attr = 0xff; // what if it's just that ?

	for (int i=0;i<SCREEN_W;i++) { // column char
		// draw a character on this line
		uint8_t p = font88_data_cached[(uint8_t) vram[vga_line / 8][i]][vga_line%8];
		uint8_t attr = vram_attr[vga_line/8][i];
		if (attr != prev_attr) {
			uint32_t c = palette[attr];

			lut_data[0] = (c&0xffff)*0x10001; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<16 | c>>16); // BA
			lut_data[3] = (c>>16)*0x10001; // BB

			prev_attr = attr;
		}

		// draw a character on this line
		*dst++ = lut_data[(p>>6) & 0x3];
		*dst++ = lut_data[(p>>4) & 0x3];
		*dst++ = lut_data[(p>>2) & 0x3];
		*dst++ = lut_data[(p>>0) & 0x3];
	}
}

// --------------------------------------------------------------
#elif VGA_SIMPLE_MODE==13 // text mode 50x37 with colors (on 400x300)
extern const uint8_t font88_data[256][8];
uint8_t font88_data_cached[256][8] CCM_MEMORY;

char vram[SCREEN_H][SCREEN_W];
char vram_attr[SCREEN_H][SCREEN_W];
uint32_t palette[256]; // BG<<16 | FG couples (default values : 16c BG, 16c FG)

void graph_frame() {}
void graph_line() 
{
	uint32_t lut_data[4]; // cache couples for faster draws

	uint32_t *dst = (uint32_t*) draw_buffer;
	if (vga_odd) {
		dst += VGA_H_PIXELS/4;
	}
	uint8_t prev_attr = 0xff; // what if it's just that ?
	
	for (int i=0;i<SCREEN_W/2;i++) { // column char
		// draw a character on this line
		uint8_t p = font88_data_cached[(uint8_t) vram[vga_line / 8][vga_odd?i+SCREEN_W/2:i]][vga_line%8];
		uint8_t attr = vram_attr[vga_line/8][vga_odd?i+SCREEN_W/2:i];
		if (attr != prev_attr) {
			uint32_t c = palette[attr];

			lut_data[0] = (c&0xffff)*0x10001; // AA
			lut_data[1] = c; // AB
			lut_data[2] = (c<<16 | c>>16); // BA
			lut_data[3] = (c>>16)*0x10001; // BB

			prev_attr = attr;
		}

		// draw a character on this line
		*dst++ = lut_data[(p>>6) & 0x3];
		*dst++ = lut_data[(p>>4) & 0x3];
		*dst++ = lut_data[(p>>2) & 0x3];
		*dst++ = lut_data[(p>>0) & 0x3];
	}
}


// --------------------------------------------------------------
#endif

// --------------------------------------------------------------
// utilities


#ifdef BPP // this is a graphical mode

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
  }}

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
   #elif VGA_SIMPLE_MODE==12 ||  VGA_SIMPLE_MODE==13 
   memcpy(font88_data_cached, font88_data, sizeof(font88_data_cached));
   memset(vram_attr, 0, sizeof(vram_attr));
   palette[0] = 0x7fff0000;
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
