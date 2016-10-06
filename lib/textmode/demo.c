// test simple by default
#include "lib/textmode/textmode.h"

/* Simple text demo for color text ! 

  You can also try mode 11 in the Makefile. Be careful not to put to much color changes per line with mode 11

  To draw some text in color, draw characters to vram and color to vram_attr
  Put color definition in the palette (FG/BG to an index), then paint this index to the place you want to
  in vram_attr.

*/
const char *lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ut malesuada tellus.";

void game_init() {

    // init palette
    set_palette(0, RGB(0,255,255), RGB(0,0,128)); // cyan on blue
    set_palette(1, RGB(255,150,0), RGB(255,255,0)); // orange fg, yellow bg
    set_palette(2, RGB(255,0,0),   RGB(255,255,0)); // red FG, yellow BG
    set_palette(3, RGB(0,180,0),   RGB(0,0,128)); // green on blue

    // now a little gradient
    for (int i=0;i<8;i++) {
      set_palette(4   +i, RGB(i*32,0,0) , RGB(0,0,128));
      set_palette(4+ 8+i, RGB(0,i*32,0) , RGB(0,0,128));
      set_palette(4+16+i, RGB(0,0,i*32) , RGB(0,0,128));
    }

    // make a window with attribute 1 (orange/yellow)
    window(1,0,0,17,17);
    print_at(5,0,1,"[Ascii]");

    //  draw ascii set with attribute 2 (red/yellow)
    for (int i=0;i<256;i++) {
        vram[1+i/16][1+i%16]=i;
        vram_attr[1+i/16][1+i%16]=2;
    }

    // print text with a gradient (crudely, no \n or \t interpreting)
    for (int i=0;i<8;i++) {
      print_at(0,20+i,4+i,lorem);
    }

    print_at((SCREEN_W-16)/2,SCREEN_H-15, 0, "Hello Bitbox ");
    print_at((SCREEN_W-16)/2+14,SCREEN_H-15,2, "simple text !");

    for (int i=0;i<3;i++) 
      for (int j=0;j<8;j++) {
        print_at(20+i*4, j+4, 4+i*8+j, "   ");
      }
}


void game_frame() {
  
}
