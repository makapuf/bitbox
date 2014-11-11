#include <stdlib.h> // rand
#include "simple.h"


void game_init()
{
}

void game_frame()
{
    if (vga_frame%180==0) clear();
    for (int i=0;i<10;i++)
        draw_line(rand()%400, rand()%300, rand()%400, rand()%300, rand()%16);
}
