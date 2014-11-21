/* Test for USB devices and input events
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Textmode code stolen from 2nd stage bootloader.
 */

#include <string.h>
#include "bitbox.h"

#ifndef EMULATOR
#include "stm32f4xx.h"
#endif

#define SCRW 80

extern const uint8_t font_data [256][16];
extern char vram[];

int cx, cy;
char cbak;

void print_at(int column, int line, const char *msg)
{
	strcpy(&vram[line * SCRW + column],msg);
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
// replace with full ascii art ?
void window (int x1, int y1, int x2, int y2 )
{
	for (int i=x1+1;i<x2;i++) 
	{
		vram[y1 * SCRW + i]='\xCD';
		vram[y2 * SCRW + i]='\xCD';
	}
	for (int i=y1+1;i<y2;i++) 
	{
		vram[i * SCRW + x1]='\xBA';
		vram[i * SCRW + x2]='\xBA';
	}
	vram[y1 * SCRW + x1] ='\xC9';
	vram[y1 * SCRW + x2] ='\xBB'; 
	vram[y2 * SCRW + x1] ='\xC8'; 
	vram[y2 * SCRW + x2] ='\xBC'; 
}


void game_init() {
	window(2,2,78,4);
	print_at(17,3, " BITBOX USB TEST \x01 Hi ! Plug some usb device...");
	print_at(1, 5, "Gamepad:");
	print_at(1, 6, "Mouse: X=   Y=   lmr");

	cx = cy = 0;
}


void display_first_event(void)
{
#ifndef EMULATOR // waiting for event_get/event_push support in emulator ;)
	static const char *HEX_Digits = "0123456789ABCDEF";

	struct event e;
	e=event_get();
	while (e.type != no_event) {
		switch(e.type)
		{
			case evt_keyboard_press : 
				print_at(50,10,"KB pressed      ");
				vram[10 * SCRW + 61]=HEX_Digits[e.kbd.key>>4];
				vram[10 * SCRW + 62]=HEX_Digits[e.kbd.key&0xf];
				break;

			case evt_keyboard_release : 
				print_at(50,10,"KB released     ");
				vram[10 * SCRW + 63]=HEX_Digits[e.kbd.key>>4];
				vram[10 * SCRW + 64]=HEX_Digits[e.kbd.key&0xf];
				break;

			case evt_device_change:
				// It seems the disconnect event is not sent currently...
				if (e.device.type == device_unconnected)
					print_at(50, 10, "dev. disconnect");
				else if (e.device.type == device_keyboard)
					print_at(50, 10, "Keyboard found!");
				else if (e.device.type == device_mouse)
					print_at(50, 10, "Mouse found!");
				else if (e.device.type == device_gamepad)
					print_at(50, 10, "Gamepad found!");
				break;

			case evt_mouse_move:
				vram[6 * SCRW + 10]=HEX_Digits[e.mov.x>>4];
				vram[6 * SCRW + 11]=HEX_Digits[e.mov.x&0xf];

				vram[6 * SCRW + 15]=HEX_Digits[e.mov.y>>4];
				vram[6 * SCRW + 16]=HEX_Digits[e.mov.y&0xf];

				cx += e.mov.x;
				cy += e.mov.y;
				if (cy < 0) cy = 480;
				else if (cy >= 480) cy = 0;
				if (cx < 0) cx = 640;
				else if (cx >= 640) cx = 0;
				break;

			case evt_mouse_click:
				// the buttons IDs are NOT mousebut_left/right/middle...
				if (e.button.id == 0) vram[6 * SCRW + 18] = 'L';
				if (e.button.id == 2) vram[6 * SCRW + 19] = 'M';
				if (e.button.id == 1) vram[6 * SCRW + 20] = 'R';
				break;

			case evt_mouse_release:
				if (e.button.id == 0) vram[6 * SCRW + 18] = 'l';
				if (e.button.id == 2) vram[6 * SCRW + 19] = 'm';
				if (e.button.id == 1) vram[6 * SCRW + 20] = 'r';
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
#endif
}

void game_frame() 
{
	vram[(cy / 16) * SCRW + cx / 8] = cbak;

	display_first_event();

	char* val = &vram[5 * SCRW + 10];

	// handle input 
	val[0] = GAMEPAD_PRESSED(0,up) ? 'U':'u';
	val[1] = GAMEPAD_PRESSED(0,down) ? 'D':'d';
	val[2] = GAMEPAD_PRESSED(0,left) ? 'L':'l';
	val[3] = GAMEPAD_PRESSED(0,right) ? 'R':'r';

	val[5] = GAMEPAD_PRESSED(0,A) ? 'A':'a';
	val[6] = GAMEPAD_PRESSED(0,B) ? 'B':'b';
	val[7] = GAMEPAD_PRESSED(0,X) ? 'X':'x';
	val[8] = GAMEPAD_PRESSED(0,Y) ? 'Y':'y';

	val[10] = GAMEPAD_PRESSED(0,L) ? 'L':'l';
	val[11] = GAMEPAD_PRESSED(0,R) ? 'R':'r';
	val[12] = GAMEPAD_PRESSED(0,select) ? 'S':'s';
	val[13] = GAMEPAD_PRESSED(0,start) ? 'G':'g';

	cbak = vram[(cy / 16) * SCRW + cx / 8];
	vram[(cy / 16) * SCRW + cx / 8] = 127;

}

