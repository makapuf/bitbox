#include <stdlib.h> // rand
#include "lib/simple.h"

void game_init()
{
}

void game_frame()
{
    if (vga_frame%180==0) clear();
    for (int i=0;i<10;i++)
        draw_line( rand()%SCREEN_W, rand()%SCREEN_H, 
        	rand()%SCREEN_W, rand()%SCREEN_H, 
        	rand()%(1<<BPP));
}
