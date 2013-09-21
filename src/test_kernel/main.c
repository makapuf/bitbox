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
	// clear the line with a repeating red/black gradient
	for (int i=0;i<640;i++)
		draw_buffer[i]=line%0x0f; 
	
	// force pixel after screen to black.
	draw_buffer[640]=0; 
	
	// first oblique line (behind)
	for (int i=0;i<128;i++)
		draw_buffer[line+i] = (i/8)<<4;

	// square "effect"
	if ((line-frame*2)%128 <64)
		for (int i=200;i<200+256;i++) 
			draw_buffer[i]|=0x777; // you can also modify the buffer

	// second oblique line (front)
	for (int i=0;i<64;i++)
		draw_buffer[640-line-i] = (i/4)<<8;

	// display gamepad state as an inverse video point
	if (line==200+y)
	{
		draw_buffer[320+x]^=0xfff;
	}
}

void game_sample() {}