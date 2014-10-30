#include <bitbox.h>

volatile int x,y, snd_vol;

// x and y  should be volatile since the vga thread calling line must see the changes to x and y 
// which are runnin in the main thread 

void game_init() {
	audio_on=1;
	snd_vol=0; // 0-31 volume
}

void game_frame()
{

    if (GAMEPAD_PRESSED(0,up) && y>-240) y--;
    if (GAMEPAD_PRESSED(0,down) && y<240) y++;
    if (GAMEPAD_PRESSED(0,left) && x>-320) x--;
    if (GAMEPAD_PRESSED(0,right) && x<320) x++;

    if (gamepad_buttons[0]) {
    	snd_vol=32;
    } else {
    	if (snd_vol) 
    		snd_vol--;
    }
} 

void graph_frame(){}
void graph_line()
// called from VGA kernel
{	

	// last 15 lines as RGB bits
	if (vga_line>=480-30){
		for (int i=0;i<640;i++)
			draw_buffer[i] = 1<<((vga_line-(480-30))/2);
		return;		
	}

	// clear the line with a repeating gradient
	for (int i=0;i<VGA_H_PIXELS;i++)
		draw_buffer[i]=(vga_line&0x1f)<<(5*((vga_line/32)&3)); 
	
	// force pixel after screen to black.
	draw_buffer[VGA_H_PIXELS]=0; 
	
	// first oblique line (behind)
	for (int i=0;i<128;i++)
		draw_buffer[vga_line+i] = (i/8)<<4;

	// square "effect"
	if ((vga_line-vga_frame*2)%128 <64)
		for (int i=200;i<200+128;i++) 
			draw_buffer[i]|=((vga_frame/4)%16)*0x421; // you can also modify the buffer

	// second oblique line (front)
	for (int i=0;i<64;i++)
		draw_buffer[VGA_H_PIXELS-vga_line-i] = (i/4)<<8;

    // display gamepad state as 16 inverse video pixels
    if (vga_line == 200) {
    for (int i=0; i<16; i++)
      if (gamepad_buttons[0] & (1 << i)) draw_buffer[320+i]^=0x7fff;
    }

    if (vga_line==200+y)
	{
		draw_buffer[320+x]^=0x7fff;
	}

	// display mouse state as crosshair
	if (vga_line==data_mouse_y) {
		for (int i=-2;i<=2;i++)	draw_buffer[data_mouse_x+i] ^= 0x7fff; 
	}
	if (vga_line >= data_mouse_y-2 && vga_line<=2+data_mouse_y) {
		draw_buffer[data_mouse_x] ^= 0x7fff; 
	}
}

void game_snd_buffer(uint16_t *buffer, int len) 
/* generates a 1kHz sound alternatibng between left & right */
{
	for (int i=0;i<len;i++)
	{
		if (i&32) // square each 32 samples 
		{
			*buffer++ = snd_vol*((vga_frame/8)&1 ?  0x4000 : 0x0040)/32;
		} else {
			*buffer++ = 0;
		}
	}
}
