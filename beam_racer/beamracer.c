/* BEAM RACER
 * Copyright 2014, Adrien Destugues <pulkomandy@gmail.com>
 * This program is distributed under the terms of the MIT License
 */

#include "bitbox.h"
#include <string.h>

extern uint8_t roadtex[256][180];

#define red RGB(255, 0, 0)
#define white RGB(255, 255, 255)
#define green RGB(0, 255, 0)
#define grey RGB(63, 63, 63)

static const uint32_t paletteA[16] = {
	red | (red << 16),white | (red << 16), green | (red << 16), grey | (red << 16),
	red | (white << 16), white | (white << 16), green | (white << 16), grey | (white << 16),
	red | (green << 16), white | (green << 16), green | (green << 16), grey | (green << 16),
	red | (grey << 16), white | (grey << 16), green | (grey << 16), grey | (grey << 16),
};

#define red2 RGB(255, 255, 255)
#define white2 RGB(63, 63, 63)
#define green2 RGB(0, 127, 0)
#define grey2 RGB(63, 63, 63)
static const uint32_t paletteB[16] = {
	red2 | (red2 << 16), white2 | (red2 << 16), green2 | (red2 << 16), grey2 | (red2 << 16),
	red2 | (white2 << 16), white2 | (white2 << 16), green2 | (white2 << 16), grey2 | (white2 << 16),
	red2 | (green2 << 16), white2 | (green2 << 16), green2 | (green2 << 16), grey2 | (green2 << 16),
	red2 | (grey2 << 16), white2 | (grey2 << 16), green2 | (grey2 << 16), grey2 | (grey2 << 16),
};

void game_init() {};
void game_frame() {};
void game_line() {};
void game_snd_buffer(uint16_t* buffer, int len) {};

static int r, dr, ddr; // Steering pos / speed / acceleration
static int horizon, q, dq, ddq;

void graph_frame() {
	dr = 0;
	ddr = (data_mouse_x - 160) / 2;
	r = (-200 * ddr + 10 * 256) * 128;

	q = 0;

	if (data_mouse_buttons & mousebut_left) {
		dq = 1 << 14;
		ddq = 100;
		horizon = 200;
	} else if (data_mouse_buttons & mousebut_right) {
		dq = 1<<16;
		ddq = -300;
		horizon = 256;
	} else {
		horizon = 240;
		dq = 1 << 15;
		ddq = 0;
	}
};

void graph_line() {
	// 64bit access to line buffer for faster blitting
	uint64_t* buf = (uint64_t*)draw_buffer;


	if (vga_line < horizon) {
		// Draw the sky
		uint64_t color = RGB(vga_line + 16, 255, 255);
		color = color | (color << 16);
		color = color | (color << 32);
		for (int word = 0; word < 80; word++) {
			*(buf++) = color;
		}

		// TODO add some background/Horizon image (should move on hills and
		// when steering, too)
	} else {
		// Draw the road
		// Compute stripes state
		int z;
		const int speed = 48;
		const int height = (256 * 1024);
		const int stripe = 512;

		dq += ddq;
		q += dq;
		if (q >> 15 > 255) q = 255 << 15;

		z = height / ((q >> 15) + 32);
		z += vga_frame * speed;

		// Compute road curve
		dr += ddr;
		r += dr;

		const uint32_t* p = ((z + vga_frame * speed) & stripe) ? paletteA : paletteB;
		uint32_t* buf = (uint32_t*)draw_buffer;
		for (int px = 0; px < 160; px++) {
			buf[px] = p[roadtex[q >> 15][(r >> 15) + px]];
		}
	}

	// Draw sprites
	if (vga_line > 480 - 140) {
		extern uint16_t carsprite[70][80];
		uint16_t* sl = carsprite[(vga_line - 340) / 2];
		for (int i = 0; i < 80; i++) {
			if (sl[i] != 0x7FFF) draw_buffer[114 + i] = sl[i];
		}
	}
};
