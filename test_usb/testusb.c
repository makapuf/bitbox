/* Test for USB devices and input events
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Textmode code stolen from 2nd stage bootloader.
 */

#include <string.h>
#include "bitbox.h"
#include "simple.h"

int cx, cy;

void game_init() {
	clear(); 
	
	window(2,2,78,4);
	print_at(17,3, " BITBOX USB TEST \x01 Hi ! Plug some usb device...");
	print_at(1, 6, "Mouse: X=   Y=   lmr");

	print_at(1, 7, "Gamepad:");
	// "graphical" gamepad
	window(8, 8, 8+17, 12);
	print_at(10,8,"[ ]");
	print_at(4+17,8,"[ ]");

	// analog values
	window(27, 6, 27+17, 6+9);
	print_at(28,5,"Analog pad:   x");

	cx = cy = 0;
}


static const char *HEX_Digits = "0123456789ABCDEF";
void display_first_event(void)
{
	struct event e;
	e=event_get();
	while (e.type != no_event) {
		switch(e.type)
		{
			case evt_keyboard_press : 
				print_at(1,15,"KB pressed      ");
				vram[15][14]=HEX_Digits[(e.kbd.key>>4) & 0xF];
				vram[15][15]=HEX_Digits[e.kbd.key&0xf];
				vram[15][17]=e.kbd.sym;
				break;

			case evt_keyboard_release : 
				print_at(1,15,"KB released     ");
				vram[15][14]=HEX_Digits[(e.kbd.key>>4) & 0xF];
				vram[15][15]=HEX_Digits[e.kbd.key&0xf];
				vram[15][17]=e.kbd.sym;
				break;

			case evt_device_change:
				// It seems the disconnect event is not sent currently...
				if (e.device.type == device_unconnected)
					print_at(1, 15, "dev. disconnect");
				else if (e.device.type == device_keyboard)
					print_at(1, 15, "Keyboard found!");
				else if (e.device.type == device_mouse)
					print_at(1, 15, "Mouse found!");
				else if (e.device.type == device_gamepad)
					print_at(1, 15, "Gamepad found!");
				break;

			case evt_mouse_move:
				vram[6][10]=HEX_Digits[(e.mov.x>>4) & 0xF];
				vram[6][11]=HEX_Digits[e.mov.x&0xf];

				vram[6][15]=HEX_Digits[(e.mov.y>>4) & 0xF];
				vram[6][16]=HEX_Digits[e.mov.y&0xf];

				cx += e.mov.x;
				cy += e.mov.y;
				if (cy < 0) cy = 480;
				else if (cy >= 480) cy = 0;
				if (cx < 0) cx = 640;
				else if (cx >= 640) cx = 0;
				break;

			case evt_mouse_click:
				// the buttons IDs are NOT mousebut_left/right/middle...
				if (e.button.id == 0) vram[6][18] = 'L';
				if (e.button.id == 2) vram[6][19] = 'M';
				if (e.button.id == 1) vram[6][20] = 'R';
				break;

			case evt_mouse_release:
				if (e.button.id == 0) vram[6][18] = 'l';
				if (e.button.id == 2) vram[6][19] = 'm';
				if (e.button.id == 1) vram[6][20] = 'r';
				break;

			case evt_user:
				print_at(50, 10, "user event");
				break;

			case no_event:
				break;

			default:
				print_at(50, 10, "UNHANDLED");
		}
		e=event_get();
	}
	//if (e.type) event_push(e); // put it back
}

void game_frame() 
{
	static char cbak;
	static int8_t gpx, gpy;

	vram[cy / 16][cx / 8] = cbak;

	display_first_event();

	// gamepad buttons
	vram[9][11] = GAMEPAD_PRESSED(0,up) ? 'U':'u';
	vram[11][11] = GAMEPAD_PRESSED(0,down) ? 'D':'d';
	vram[10][9] = GAMEPAD_PRESSED(0,left) ? 'L':'l';
	vram[10][13] = GAMEPAD_PRESSED(0,right) ? 'R':'r';

	vram[10][24] = GAMEPAD_PRESSED(0,A) ? 'A':'a';
	vram[11][22] = GAMEPAD_PRESSED(0,B) ? 'B':'b';
	vram[9][22] = GAMEPAD_PRESSED(0,X) ? 'X':'x';
	vram[10][20] = GAMEPAD_PRESSED(0,Y) ? 'Y':'y';

	vram[8][11] = GAMEPAD_PRESSED(0,L) ? 'L':'l';
	vram[8][22] = GAMEPAD_PRESSED(0,R) ? 'R':'r';
	vram[11][15] = GAMEPAD_PRESSED(0,select) ? 'S':'s';
	vram[11][18] = GAMEPAD_PRESSED(0,start) ? 'G':'g';

	// analog gamepad
	vram[5][40]=HEX_Digits[(gamepad_x[0]>>4) & 0xF];
	vram[5][41]=HEX_Digits[gamepad_x[0]&0xf];

	vram[5][43]=HEX_Digits[(gamepad_y[0]>>4) & 0xF];
	vram[5][44]=HEX_Digits[gamepad_y[0]&0xf];

	vram[11 + gpy / 32][36 + gpx / 16] = ' ';

	gpx = gamepad_x[0];
	gpy = gamepad_y[0];

	vram[11 + gpy / 32][36 + gpx / 16] = '+';

	// mouse cursor
	cbak = vram[cy / 16][cx / 8];
	vram[cy / 16][cx / 8] = 127;

}

