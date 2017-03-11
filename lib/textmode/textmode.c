#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#include "textmode.h"
// make sure to use CCM_MEMORY instead of .ccm for Mac based builds...

#define PASTE(a,b,c,d,e) a##b##c##d##e
#define PASTE2(a,b,c,d,e) PASTE(a,b,c,d,e) // recursively expanded argument
#define FONTDATA PASTE2(font,FONT_W,x,FONT_H,_data) // e.g. will produce font8x16_data

#include "fonts.h"

extern const uint8_t FONTDATA[256][FONT_H];
uint8_t font_data_cached[256][FONT_H] CCM_MEMORY;

uint8_t font_cached=0; 

char vram[SCREEN_H][SCREEN_W];
uint8_t vram_attr[SCREEN_H][SCREEN_W];

uint32_t palette[64][4]; // cache of BG<<16 | FG couples - AA,AB,BA,BB 

inline void set_palette(uint8_t pen,uint16_t col, uint16_t bg )
{
	uint32_t *lut=&palette[pen&63][0];
	lut[3] = col<<16|col;
	lut[2] =  bg<<16|col;
	lut[1] = col<<16|bg;
	lut[0] =  bg<<16|bg; 
}

void graph_vsync() {
	// load font data to RAM cache
	if (!font_cached && vga_line==VGA_V_PIXELS+3) {
		font_cached=1;
 	    memcpy(font_data_cached, FONTDATA, sizeof(font_data_cached));
	}
}

void graph_line() {
	if (vga_odd) return;
	
	uint32_t *dst = (uint32_t*) draw_buffer; // will blit 2 by 2

	for (int i=0;i<SCREEN_W;i++) { // column char
		// draw a character on this line
		uint8_t p = font_data_cached[(uint8_t) vram[vga_line / FONT_H][i]][vga_line%FONT_H];
		uint8_t attr = vram_attr[vga_line/FONT_H][i];
		uint32_t *lut_data = &palette[attr%64][0];

		// draw a character on this line
		switch(FONT_W) {
			case 8:
				*dst++ = lut_data[(p>>6) & 0x3];
				// fallthrough
			case 6:
				*dst++ = lut_data[(p>>4) & 0x3];
				// fallthrough
			case 4:
				*dst++ = lut_data[(p>>2) & 0x3];
				*dst++ = lut_data[(p>>0) & 0x3];
		}
	}
}


// --------------------------------------------------------------
// utilities


void clear()
{
   memset(vram, 0, sizeof(vram));
   memset(vram_attr, 0, sizeof(vram_attr));
   set_palette(0,0x7fff,0x0000); // white on black
}

int print_at(int column, int line, uint8_t pen, const char *msg)
{
	int k;
	for(k = 0; msg[k] && (k+column+line*SCREEN_W<SCREEN_H*SCREEN_W); k++) {
		vram[line][column + k] = msg[k];
		vram_attr[line][column + k] = pen;
  	}
  	return k;
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (uint8_t pen, int x1, int y1, int x2, int y2)
{
	for (int i=x1+1;i<x2;i++)
	{
		vram[y2][i]=vram[y1][i]='\xCD';
	    vram_attr[y1][i]=vram_attr[y2][i]=pen;

	}
	for (int i=y1+1;i<y2;i++)
	{
		vram[i][x1]=vram[i][x2]='\xBA';
    	vram_attr[i][x1]=vram_attr[i][x2]=pen;
	}

	vram[y1][x1] ='\xC9';
	vram[y1][x2] ='\xBB';
	vram[y2][x1] ='\xC8';
	vram[y2][x2] ='\xBC';

	vram_attr[y1][x1] =vram_attr[y1][x2]=vram_attr[y2][x1]=vram_attr[y2][x2]=pen;
}
