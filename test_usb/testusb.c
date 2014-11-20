/* Test for USB devices and input events
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Textmode code stolen from 2nd stage bootloader.
 */

#include <string.h>
#include "bitbox.h"

#ifndef EMULATOR
#include "stm32f4xx.h"
#endif

extern const uint8_t font_data [256][16];
char vram_char  [30][80];

int cx, cy;
char cbak;

void print_at(int column, int line, const char *msg)
{
	strcpy(&vram_char[line][column],msg); 
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
// replace with full ascii art ?
void window (int x1, int y1, int x2, int y2 )
{
	for (int i=x1+1;i<x2;i++) 
	{
		vram_char[y1][i]='\xCD';
		vram_char[y2][i]='\xCD';
	}
	for (int i=y1+1;i<y2;i++) 
	{
		vram_char[i][x1]='\xBA';
		vram_char[i][x2]='\xBA';
	}
	vram_char[y1][x1] ='\xC9';
	vram_char[y1][x2] ='\xBB'; 
	vram_char[y2][x1] ='\xC8'; 
	vram_char[y2][x2] ='\xBC'; 
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
				vram_char[10][61]=HEX_Digits[e.kbd.key>>4];
				vram_char[10][62]=HEX_Digits[e.kbd.key&0xf];
				break;

			case evt_keyboard_release : 
				print_at(50,10,"KB released     ");
				vram_char[10][63]=HEX_Digits[e.kbd.key>>4];
				vram_char[10][64]=HEX_Digits[e.kbd.key&0xf];
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
				vram_char[6][10]=HEX_Digits[e.mov.x>>4];
				vram_char[6][11]=HEX_Digits[e.mov.x&0xf];

				vram_char[6][15]=HEX_Digits[e.mov.y>>4];
				vram_char[6][16]=HEX_Digits[e.mov.y&0xf];

				cx += e.mov.x;
				cy += e.mov.y;
				if (cy < 0) cy = 480;
				else if (cy >= 480) cy = 0;
				if (cx < 0) cx = 640;
				else if (cx >= 640) cx = 0;
				break;

			case evt_mouse_click:
				// the buttons IDs are NOT mousebut_left/right/middle...
				if (e.button.id == 0) vram_char[6][18] = 'L';
				if (e.button.id == 2) vram_char[6][19] = 'M';
				if (e.button.id == 1) vram_char[6][20] = 'R';
				break;

			case evt_mouse_release:
				if (e.button.id == 0) vram_char[6][18] = 'l';
				if (e.button.id == 2) vram_char[6][19] = 'm';
				if (e.button.id == 1) vram_char[6][20] = 'r';
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
	vram_char[cy / 16][cx / 8] = cbak;

	display_first_event();

	char* val = &vram_char[5][10];

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

	cbak = vram_char[cy / 16][cx / 8];
	vram_char[cy / 16][cx / 8] = 127;

}



extern const uint16_t bg_data[256];

void graph_frame() {}

void graph_line() 
{
	static uint32_t lut_data[4];
	// text mode

	// bg effect, just because
	uint16_t line_color = bg_data[(vga_frame+vga_line)%256];
	lut_data[0] = 0;
	lut_data[1] = line_color<<16;
	lut_data[2] = (uint32_t) line_color;
	lut_data[3] = line_color * 0x10001;
	

	uint32_t *dst = (uint32_t *) draw_buffer;
	char c;

	for (int i=0;i<80;i++) // column char
	{
		c = font_data[(uint8_t)vram_char[vga_line / 16][i]][vga_line%16];
		// draw a character on this line

		*dst++ = lut_data[(c>>6) & 0x3];
		*dst++ = lut_data[(c>>4) & 0x3];
		*dst++ = lut_data[(c>>2) & 0x3];
		*dst++ = lut_data[(c>>0) & 0x3];
	}
}

void game_snd_buffer(uint16_t *buffer, int len) {} // beeps ?

