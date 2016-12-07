/* Simple color Text modes for the bitbox - 16bit interface
 * configure by defining a video mode and a font by defining FONT_W to and FONT_H to (8,16), (8,8) or (6,8)
 * number of characters will vary along chosen video mode 
 */

#include "bitbox.h"

// defaults to 8x8
#ifndef FONT_W
#define FONT_W 8
#define FONT_H 8
#endif 

#define SCREEN_W (VGA_H_PIXELS/FONT_W)
#define SCREEN_H ((VGA_V_PIXELS+FONT_H-1)/FONT_H)

// Utilities ------------------------------------------------

extern char vram[SCREEN_H][SCREEN_W];
extern char vram_attr[SCREEN_H][SCREEN_W]; // 0-63 and loops

void clear();
void set_palette(uint8_t pen, uint16_t color, uint16_t background); // for 8bit, use values <256

// Print a string. Returns the number of char actually printed
int print_at(int column, int line, uint8_t pen, const char *msg); 

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (uint8_t pen, int x1, int y1, int x2, int y2);
