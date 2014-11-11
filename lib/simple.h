// simple.h : a simple frmebuffer based engine
#include "bitbox.h"

/* 
    modes 
    0 - txt 16c 80x25  fg/bg  (16x8 chars)	4k vram + 4k ROM bitmap 
    1 - txt 16c 132x75 (8x6 chars) 	19k vram+2k ROM bitmap 	

    2 - 1BPP 800x600   
    3 - 2BPP 640x400   
    4 - 4BPP 400x300   
    5 - 8BPP 320x200   
*/

#if   VGA_SIMPLE_MODE==0 
#define SCREEN_W 80
#define SCREEN_H 25

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

#endif


#ifdef BPP // only for graphical modes
extern uint32_t vram[];
extern uint16_t palette[];

void draw_pixel(int x, int y, int c);
void draw_line(int x0, int y0, int x1, int y1, int c);
void clear();
#else
extern char vram[];
#endif
