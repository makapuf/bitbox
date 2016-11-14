// simple.h : a simple frmebuffer based engine
#include "bitbox.h"

/*
    modes

    0 - b&w text 80x30  fg/bg  (16x8 chars)	4k vram + 4k ROM bitmap
    1 - b&w text 132x75 (6x8 chars) 	19k vram+2k ROM bitmap
    10 - color text 80x30
    11 - color text 120x75
    12 - color text 80x60

    [ 64k modes ]

    2 - 1BPP 800x600
    3 - 2BPP 640x400
    4 - 4BPP 400x300 - avail on micro
    5 - 8BPP 320x200 - avail on micro

    6 - 16BPP 200x150 - TODO

    [ 32k modes ]

    7 - 2BPP 400x300 2bpp - TODO
    8 - 4BPP 320x200 - avail on micro

    13 - color textmode 50x37 ( based on 400x300 mode with 8x8 chars ) - avail on micro
    14 - color textmode 40x30 (320x240 with 6x8 chars) - TODO

*/

#if   VGA_SIMPLE_MODE==0
#define SCREEN_W 80
#define SCREEN_H 30

#elif VGA_SIMPLE_MODE==1
#define SCREEN_W 132
#define SCREEN_H 75

#elif VGA_SIMPLE_MODE==2
#define SCREEN_W 800
#define SCREEN_H 600
#define BPP 1

#elif VGA_SIMPLE_MODE==3
#define SCREEN_W 640
#define SCREEN_H 400
#define BPP 2

#elif VGA_SIMPLE_MODE==4
#define SCREEN_W 400
#define SCREEN_H 300
#define BPP 4

#elif VGA_SIMPLE_MODE==5
#define SCREEN_W 320
#define SCREEN_H 200
#define BPP 8
/*
#elif VGA_SIMPLE_MODE==6
#define SCREEN_W 200
#define SCREEN_H 150
#define BPP 16

#elif VGA_SIMPLE_MODE==7
#define SCREEN_W 400
#define SCREEN_H 300
#define BPP 2
*/
#elif VGA_SIMPLE_MODE==8
#define SCREEN_W 320
#define SCREEN_H 200
#define BPP 4

#elif VGA_SIMPLE_MODE==10
#define SCREEN_W 80
#define SCREEN_H 30
#define COLOR_TEXT

#elif VGA_SIMPLE_MODE==11
#define SCREEN_W 120
#define SCREEN_H 75
#define COLOR_TEXT

#elif VGA_SIMPLE_MODE==12
#define SCREEN_W 80
#define SCREEN_H 60
#define COLOR_TEXT

#elif VGA_SIMPLE_MODE==13
#define SCREEN_W 50
#define SCREEN_H 38
#define COLOR_TEXT
#else
#warning UNKNOWN Simple MODE  ! use 0-8 or 10-13
#endif

// Utilities ------------------------------------------------

void clear();

#ifdef BPP // only for graphical modes
extern uint32_t vram[];

#ifdef BOARD_MICRO
extern uint8_t palette[];
#else
extern uint16_t palette[];
#endif

void draw_pixel(int x, int y, int c);
void draw_line(int x0, int y0, int x1, int y1, int c);

#else // only for text modes

extern char vram[SCREEN_H][SCREEN_W];

#ifdef COLOR_TEXT  // color attributes
extern char vram_attr[SCREEN_H][SCREEN_W];

#ifdef BOARD_MICRO
extern uint16_t palette[];
#else
extern uint32_t palette[];
#endif

extern uint8_t text_color;
#endif

// Print a string. Returns the number of char actually printed.
int print_at(int column, int line, const char *msg);

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (int x1, int y1, int x2, int y2);

#endif
