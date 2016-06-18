// test simple by default
#include "simple.h"

/* Simple text demo for color text ! 

  You can also try mode 11 in the Makefile. Be careful not to put to much color changes per line with mode 11

  To draw some text in color, draw characters to vram and color to vram_attr
  Put color definition in the palette (FG/BG to an index), then paint this index to the place you want to
  in vram_attr.

*/
const char *lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ut malesuada tellus.";
void game_init() {
    clear();

    #ifndef BOARD_MICRO
    // init palette
    palette[0]=RGB(0,255,255)<<16|RGB(0,0,128); // cyan on blue
    palette[1]=RGB(255,150,0)<<16|RGB(255,255,0); // orange fg, yellow bg
    palette[2]=RGB(255,0,0)<<16|RGB(255,255,0); // red FG, yellow BG
    palette[3]=RGB(0,180,0)<<16|RGB(0,0,128); // green on blue
    // now a little gradient
    for (int i=0;i<8;i++) {
      palette[4+i] = RGB(i*32,0,0)| RGB(0,0,128)<<16;
      palette[4+8+i] = RGB(0,i*32,0) | RGB(0,0,128)<<16;
      palette[4+16+i] = RGB(0,0,i*32)| RGB(0,0,128)<<16;
    }

    #else
    palette[0]=RGB8(0,255,255)<<8|RGB8(0,0,128); // cyan on blue
    palette[1]=RGB8(255,150,0)<<8|RGB8(255,255,0); // orange fg, yellow bg
    palette[2]=RGB8(255,0,0)<<8|RGB8(255,255,0); // red FG, yellow BG
    palette[3]=RGB8(0,180,0)<<8|RGB8(0,0,128); // green on blue
    // now a little gradient
    for (int i=0;i<8;i++) {
      palette[4+i] = i*32 ; // reds
      palette[4+8+i] = (uint8_t[]){8,16,24,25}[i/2]; // greens
      palette[4+16+i] = (uint8_t[]){2,4,6,7}[i/2]; // blues
    }   
    #endif 
    

    // make a window with attribute 1 (orange/yellow)
    text_color=1;
    window(0,0,17,17);
    print_at(5,0,"[Ascii]");

    //  draw ascii set with attribute 2 (red/yellow)
    for (int i=0;i<256;i++) {
        vram[1+i/16][1+i%16]=i;
        vram_attr[1+i/16][1+i%16]=2;
    }

    // print text with a gradient (crudely, no \n or \t interpreting)
    for (int i=0;i<8;i++) {
      text_color=4+i;
      print_at(0,20+i,lorem);
    }

    text_color=0;
    print_at((SCREEN_W-16)/2,SCREEN_H-15, "Hello Bitbox ");
    text_color=2;
    print_at((SCREEN_W-16)/2+14,SCREEN_H-15, "simple text !");

    for (int i=0;i<3;i++) 
      for (int j=0;j<8;j++)
      {
        text_color=4+i*8+j;
        print_at(20+i*4,j+4,"   ");
      }
}


void game_frame() {
  
}
