#include <kernel.h>

volatile int x,y;
// x and y  should be volatile since the vga thread must see the changes to x and y 
// which are runnin in the main thread 

void game_init() {}

void game_frame()
{

    if (PRESSED(up) && y>-240) y--;
    if (PRESSED(down) && y<240) y++;
    if (PRESSED(left) && x>-320) x--;
    if (PRESSED(right) && x<320) x++;

} 

void game_line()
// called from VGA kernel
{	
	// clear the line with a repeating gradient
	for (int i=0;i<LINE_LENGTH;i++)
		draw_buffer[i]=(vga_line&0x1f)<<(5*((vga_line/32)&3)); 
	
	// force pixel after screen to black.
	draw_buffer[LINE_LENGTH]=0; 
	
	// first oblique line (behind)
	for (int i=0;i<128;i++)
		draw_buffer[vga_line+i] = (i/8)<<4;

	// square "effect"
	if ((vga_line-vga_frame*2)%128 <64)
		for (int i=200;i<200+128;i++) 
			draw_buffer[i]|=((vga_frame/4)%16)*0x421; // you can also modify the buffer

	// second oblique line (front)
	for (int i=0;i<64;i++)
		draw_buffer[LINE_LENGTH-vga_line-i] = (i/4)<<8;

    // display gamepad state as an inverse video point
   
    if (vga_line == 200) {
    for (int i=0; i<16; i++)
      if (gamepad1 & (1 << i)) draw_buffer[320+i]^=0x7fff;
    }
    	
    if (vga_line==200+y)
	{
		draw_buffer[320+x]^=0x7fff;
	}
}

void game_snd_buffer(uint8_t *buffer, int len) {};
