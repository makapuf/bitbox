#include <stdlib.h> // rand
#include "bitbox.h"
#include "lib/framebuffer/framebuffer.h"

void game_init()
{
}

void game_frame()
{
    if (vga_frame%180==0) clear();
    for (int i=0;i<10;i++)
        draw_line( rand()%VGA_H_PIXELS, rand()%VGA_V_PIXELS, 
        	rand()%VGA_H_PIXELS, rand()%VGA_V_PIXELS, 
        	rand()%(1<<FRAMEBUFFER_BPP));
}
