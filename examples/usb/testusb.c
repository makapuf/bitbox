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
#define KB_Y 20 // keyboard press positions
int cx, cy;

void game_init() {
	clear(); 
	
	window(0,2,1,45,4);
	print_at(14,2,0, " \xf9\xfa\xfb USB TEST ");
	print_at(5,3,0,  " \x01 Hi ! Plug some usb device...");
	print_at(2, 6, 0,"Mouse: X=   Y=   lmr");

	print_at(PAD_X+2,PAD_Y-1,0, "Gamepad:");
	// "graphical" gamepad
	window(0,PAD_X, PAD_Y, PAD_X+17, PAD_Y+5);
	print_at(PAD_X+2 ,PAD_Y,0,"[ ]");
	print_at(PAD_X+13,PAD_Y,0,"[ ]");

	// analog values
	window(0,27, 6, 27+17, 6+9);
	print_at(28,5,0,"Analog pad:   x");

	cx = cy = 0;
}


static const char *HEX_Digits = "0123456789ABCDEF";
static const char *KBMOD="CSAWCSAW";

void game_frame() 
{
	static char cbak;
	static int8_t gpx, gpy;

	// mouse
	vram[cy / 16][cx / 8] = cbak;

	cy += mouse_y;
	if (cy < 0) cy = 480;
	else if (cy >= 480) cy = 0;
	
	cx += mouse_x; 
	if (cx < 0) cx = 640;
	else if (cx >= 640) cx = 0;

	vram[6][11]=HEX_Digits[(cx>>4) & 0xF];
	vram[6][12]=HEX_Digits[cx&0xf];

	vram[6][16]=HEX_Digits[(cy>>4) & 0xF];
	vram[6][17]=HEX_Digits[cy&0xf];

	cbak = vram[cy / 16][cx / 8];
	vram[cy / 16][cx / 8] = 127;

	vram[6][19]=mouse_buttons & mousebut_left?'L':'l';
	vram[6][20]=mouse_buttons & mousebut_middle?'M':'m';
	vram[6][21]=mouse_buttons & mousebut_right?'R':'r';

	// gamepad buttons
	vram[PAD_Y+1][PAD_X+3] = GAMEPAD_PRESSED(0,up) ? 'U':'u';
	vram[PAD_Y+3][PAD_X+3] = GAMEPAD_PRESSED(0,down) ? 'D':'d';
	vram[PAD_Y+2][PAD_X+1] = GAMEPAD_PRESSED(0,left) ? 'L':'l';
	vram[PAD_Y+2][PAD_X+5] = GAMEPAD_PRESSED(0,right) ? 'R':'r';

	vram[PAD_Y+2][PAD_X+16] = GAMEPAD_PRESSED(0,A) ? 'A':'a';
	vram[PAD_Y+3][PAD_X+14] = GAMEPAD_PRESSED(0,B) ? 'B':'b';
	vram[PAD_Y+1][PAD_X+14] = GAMEPAD_PRESSED(0,X) ? 'X':'x';
	vram[PAD_Y+2][PAD_X+12] = GAMEPAD_PRESSED(0,Y) ? 'Y':'y';

	vram[PAD_Y  ][PAD_X+ 3] = GAMEPAD_PRESSED(0,L) ? 'L':'l';
	vram[PAD_Y  ][PAD_X+14] = GAMEPAD_PRESSED(0,R) ? 'R':'r';
	vram[PAD_Y+3][PAD_X+ 9] = GAMEPAD_PRESSED(0,select) ? 'S':'s';
	vram[PAD_Y+3][PAD_X+10] = GAMEPAD_PRESSED(0,start) ? 'G':'g';

	// analog gamepad
	vram[5][40]=HEX_Digits[(gamepad_x[0]>>4) & 0xF];
	vram[5][41]=HEX_Digits[gamepad_x[0]&0xf];

	vram[5][43]=HEX_Digits[(gamepad_y[0]>>4) & 0xF];
	vram[5][44]=HEX_Digits[gamepad_y[0]&0xf];

	vram[11 + gpy / 32][36  + gpx / 16] = ' ';

	gpx = gamepad_x[0];
	gpy = gamepad_y[0];

	vram[11 + gpy / 32][36 + gpx / 16] = '+';

	// KB codes 
	for (int i=0;i<6;i++) {
		vram[21][5+i*3]=HEX_Digits[keyboard_key[0][i]>>4];
		vram[21][6+i*3]=HEX_Digits[keyboard_key[0][i]&0xf];
	}

	// KB mods
	for (int i=0;i<8;i++)
		vram[20][5+i]=keyboard_mod[0] & (1<<i) ? KBMOD[i] : '-' ;


}

