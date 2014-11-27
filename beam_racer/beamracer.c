/* BEAM RACER
 * Copyright 2014, Adrien Destugues <pulkomandy@gmail.com>
 * This program is distributed under the terms of the MIT License
 */

#include "bitbox.h"
#include <string.h>



// COLOR PALETTES -------------------------------------------------------------

// The road uses only 4 colors, but we encode 2 pixels in one byte (with 4
// unused bits currently). Then we use these palettes to lookup 2 pixels in
// only one memory access and expand them to 2 16-byte values (packed in one
// uint32_t) that we then blit on screen.
// Alternating between the two palettes at carefully chosen lines give the
// illusion of the road moving.

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

// TEXTURES -------------------------------------------------------------------

// The road texture as described above. It is wider than the screen
// (180 bytes * 2 pixels per bytes = 360 pixels) so we can offset it to the
// left or right without having part of the next line showing on the other edge.
//
// TODO: the road has a left/right symetry so we should store only half of it
// TODO: each byte has a value in 0..15 only. We could halve this and pack more
// at the cost of an extra shift&mask in the blit/lookup. Or we can make the
// road use 16 colors instead of just 4.

extern uint8_t roadtex[256][180];


// KERNEL CALLBACKS -----------------------------------------------------------

// TODO implement those to make an actual game!

void game_init() {};
void game_frame() {};
void game_line() {};

// TODO some sound and music would be great too.
void game_snd_buffer(uint16_t* buffer, int len) {};

// VIDEO CALLBACKS ------------------------------------------------------------

// Road steering position
// Steering is done by shifting the road a tiny bit left or right at each line
//
// r is the "initial" position of the road. Since we draw things top to bottom,
// this is actually the position of the road at the horizon.
//
// dr ("delta R") is the variation of r at each line ("speed" of the curve)
//
// ddr ("delta delta R") is the variation of dr at each line ("acceleration")
//
// The curves should not be very intense and we can have only at most 320 pixels
// cumulated difference between the start and end of the road. We have around
// 120 lines (half height of the screen, the rest is the sky) to do that. This
// means dr should stay in the range 0 to about 1.5, which in turn means ddr
// should be very small (as it is added to dr at each frame).
// We implement this using a fixed point scheme (r >> 10 is used as the
// position, and dr >> 10 is added to r at each frame). This gives an useful
// ddr resolution but some rounding artifacts.
static int r, dr, ddr;

// Same as above for altitude (to implement hills and valleys)
// horizon is the starting line on the screen where the road starts (make it
// higher when the car is running down a hill, and lower when it is climbing)
// q is the distorsion of the road to fake the curving. It is used in the same
// way as r with 1st and 2nd derivatives using a fixed point scheme.
static int horizon, q, dq, ddq;

void graph_frame() {
	// Curves are controlled with the mouse currently (for testing). They should
	// come from the "track" data instead.
	// We initialize ddr with the curve value (centered mouse / 2)
	ddr = (data_mouse_x - 160) / 2;

	// dr starts as 0 so the road looks straight at the horizon. This isn't
	// correct, it should start at (dr/2)² or so so the road looks straight
	// under the car wheels and distorts as it gets further
	dr = 0;

	// And this should be computed so r ends up being reasonably close to 0 on
	// the last line of the screen. Or it can be offset a bit, to make it feel
	// like the car is taking the turn and being pulled towards the outside of
	// the curve. It depends on the feeling you want to give to the game.
	// Currently the math is randomly tweaked until it feels mostly correct.
	// More mathematical way to compute that needed
	// (again, this would be simpler if the road was drawn bottom to top, but
	// asking people to flip their monitors is something you could do only in
	// arcade cabinets)
	r = (-200 * ddr + 10 * 256) * 128;

	// Compute q depending on the mouse buttons. Again, ugly hand-tweaked values
	// here, needs a more mathematical way to do this.
	// More intense values (both for q, horizon and r) will give a more
	// "rollercoaster" feel to the game. Lower variations give a more realistic
	// (but also more boring) feel.
	q = 0;

	if (data_mouse_buttons & mousebut_left) {
		dq = 1 << 14;
		ddq = 100;
		horizon = 100;
	} else if (data_mouse_buttons & mousebut_right) {
		dq = 1<<16;
		ddq = -300;
		horizon = 128;
	} else {
		horizon = 120;
		dq = 1 << 15;
		ddq = 0;
	}
};

void graph_line() {
	// 64bit access to line buffer for faster blitting
	uint64_t* buf = (uint64_t*)draw_buffer;

	// The screen is split in two parts: "above" and "below" the horizon.
	// "above" is the sky, and there can be a background image there too.
	// The background image must scroll to the left and right in curves to
	// make it look like the car is actually taking the turn.
	if (vga_line < horizon) {
		// Draw the sky - This is just a simple gradient. We could use several
		// palettes to have day and night, different weathers, and overall a
		// different look for each track.
		// There is only one palette value used for each line, so more crazy
		// effects are possible here (fading from one palette to another, or
		// a more advanced approach with color-slide math).
		uint64_t color = RGB(2 * vga_line + 16, 255, 255);

		// Once we have computed the 16-bit color, we quadruplicate (?) it in
		// a 64-bit number, which we then copy all over the screen.
		// TODO check if this is actually faster than using 32 or 16 bits.
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
		// z is the Z coordinate of the world. Assuming x and y are the usual
		// screen coordinates, z is the "distance" behind the screen the point
		// is at. We compute this by using perspective projection for a more
		// correct look.
		// (don't panic, while it is "perspective", we only need to consider
		// the y and z axes, so this is still 2D math).
		int z;

		// This is the speed at which the car is running. It should be a
		// variable and the user should control it with some gamepad buttons :)
		const int speed = 26;

		// This is the height of the user eyes. Use a lower height for an F-1
		// game in 1st-person, and an higher one for a 3rd-person game or
		// monster truck game (in both cases the eyes are higher up)
		// Note that the horizon and q values must be adjusted to match.
		const int height = (256 * 1024);

		// This is the width (in z-units) of the stripes on the road. We use a
		// multiple of 2 here so a single bit of the Z value can be used to
		// pick one of the two road palettes, making the strips visible.
		const int stripe = 512;

		// Compute the altitude for this line
		dq += ddq;
		q += dq;
		if (q >> 15 > 255) q = 255 << 15;

		// Compute Z by projecting the screen on the road according to the
		// altitude
		z = height / ((q >> 15) + 32);
		// Shift z at each frame according to the speed to make the stripes move
		z += vga_frame * speed;

		// Compute road curve
		dr += ddr;
		r += dr;

		// Now the blitting happens. The steps are:
		// Pick the correct palette depending on the z position, and current
		// frame. As mentionned above we do this with a single bit test.
		const uint32_t* p = ((z + vga_frame * speed) & stripe) ? paletteA : paletteB;

		// Get a 32-bit pointer to the screen
		uint32_t* buf = (uint32_t*)draw_buffer;
		// And do the blitting. We get 2 pixels from the road texture at once
		// according to the q (for the line) and r (offset x), look them up
		// (2 at once) using the current palette, then put that in the line
		// buffer.
		for (int px = 0; px < 160; px++) {
			buf[px] = p[roadtex[q >> 15][(r >> 15) + px]];
		}
	}

	// At this point the whole sky or road is computed. We can now render
	// sprites, and other fun stuff on it. This is done even for lines above the
	// horizon because sprites far enough on the road may be visible above the
	// horizon. For very convincing hills, even sprites further than that ("on
	// the other side of the hill") should have their top part visible above the
	// horizon
	//
	// But for now, we draw just the player's car...
	if (vga_line > 240 - 56) {
		// The car is stored as 16-bit data for direct copy currently.
		// No palette lookup here, but a "magic pink" (uh white) test to make
		// the car sprite transparent. There are better ways to handle this,
		// as the sprite is (currently) convex and symetric
		extern uint16_t carsprite[56][75];
		uint16_t* sl = carsprite[(vga_line - 184)];
		for (int i = 0; i < 75; i++) {
			if (sl[i] != RGB(255,0,255)) draw_buffer[122 + i] = sl[i];
		}
	}
};
