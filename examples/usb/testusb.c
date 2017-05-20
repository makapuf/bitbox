/* Test for USB devices and input events
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * 2016 Makapuf (split event / non events)
 */

#include <string.h>

#include "bitbox.h"
#include "lib/textmode/textmode.h"

// graphical gamepad position 
#define PAD_X 2
#define PAD_Y 10
#define PAD_Y2 15
#define KB_Y 23 // keyboard press positions

int cx, cy;

// draw "graphical" controller frame
void draw_controller(int x, int y)
{
	window(0,x, y, x+17, y+4);
	print_at(x+2 ,y,0,"[ ]");
	print_at(x+13,y,0,"[ ]");
}

void update_controller (int x, int y, uint16_t buttons)
{
	vram[y+1][x+3] = GAMEPAD_PRESSED(0,up) ? 'U':'u';
	vram[y+3][x+3] = GAMEPAD_PRESSED(0,down) ? 'D':'d';
	vram[y+2][x+1] = GAMEPAD_PRESSED(0,left) ? 'L':'l';
	vram[y+2][x+5] = GAMEPAD_PRESSED(0,right) ? 'R':'r';

	vram[y+2][x+16] = GAMEPAD_PRESSED(0,A) ? 'A':'a';
	vram[y+3][x+14] = GAMEPAD_PRESSED(0,B) ? 'B':'b';
	vram[y+1][x+14] = GAMEPAD_PRESSED(0,X) ? 'X':'x';
	vram[y+2][x+12] = GAMEPAD_PRESSED(0,Y) ? 'Y':'y';

	vram[y  ][x+ 3] = GAMEPAD_PRESSED(0,L) ? 'L':'l';
	vram[y  ][x+14] = GAMEPAD_PRESSED(0,R) ? 'R':'r';
	vram[y+3][x+ 9] = GAMEPAD_PRESSED(0,select) ? 'S':'s';
	vram[y+3][x+10] = GAMEPAD_PRESSED(0,start) ? 'G':'g';
}

void printhex(int x, int y, uint8_t n)
{
	static const char *HEX_Digits = "0123456789ABCDEF";
	vram[y][x]=HEX_Digits[(n>>4) & 0xF];
	vram[y][x+1]=HEX_Digits[n&0xf];
}

static const char *KBMOD="CSAWCSAW";

void game_init() {
	clear(); 
	
	window(0,2,1,45,4);
	print_at(14,2,0, " \xf9\xfa\xfb USB TEST ");
	print_at(5,3,0,  " \x01 Hi ! Plug some usb device...");
	print_at(2, 6, 0,"Mouse: X=   Y=   lmr");

	print_at(2,PAD_Y-2,0, "Gamepads:");
	draw_controller(PAD_X, PAD_Y);
	draw_controller(PAD_X, PAD_Y2);

	// analog values
	print_at(27,PAD_Y-2,0,"Analog pad0:   x");
	window (0,27, PAD_Y, 27+17, PAD_Y+9);

	// keyboard 
	print_at(2,KB_Y,0,"Keyboard:");

	cx = cy = 0;
}

void game_frame() 
{
	static char cbak;
	static int8_t gpx, gpy;

	// mouse
	vram[cy / 8][cx / 8] = cbak;

	cy += mouse_y; mouse_y=0;
	if (cy < 0) cy = VGA_V_PIXELS;
	else if (cy >= VGA_V_PIXELS) cy = 0;
	
	cx += mouse_x; mouse_x=0;
	if (cx < 0) cx = VGA_H_PIXELS;
	else if (cx >= VGA_H_PIXELS) cx = 0;

	printhex(11,6,cx/8);
	printhex(16,6,cy/8);

	cbak = vram[cy / 8][cx / 8];
	vram[cy / 8][cx / 8] = 127;

	vram[6][19]=mouse_buttons & mousebut_left?'L':'l';
	vram[6][20]=mouse_buttons & mousebut_middle?'M':'m';
	vram[6][21]=mouse_buttons & mousebut_right?'R':'r';

	printhex(23,6,mouse_buttons);

	// gamepad buttons
	update_controller(PAD_X, PAD_Y,gamepad_buttons[0]);
	update_controller(PAD_X, PAD_Y2,gamepad_buttons[1]);


	// analog gamepad
	printhex(40,PAD_Y-2,gamepad_x[0]);
	printhex(43,PAD_Y-2,gamepad_y[0]);

	vram[15 + gpy / 32][36  + gpx / 16] = ' ';

	gpx = gamepad_x[0];
	gpy = gamepad_y[0];
	vram[15 + gpy / 32][36 + gpx / 16] = '+';

	// KB codes 
	for (int i=0;i<6;i++) {
		printhex(5+i*3,KB_Y+2,keyboard_key[0][i]);
	}

	// KB mods
	for (int i=0;i<8;i++)
		vram[KB_Y+3][5+i]=keyboard_mod[0] & (1<<i) ? KBMOD[i] : '-' ;


}

