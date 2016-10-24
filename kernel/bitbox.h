// new lowlevel kernel base API
#pragma once

// to be provided by games or engines
#include <stdint.h>
#include "kconf.h" // kernel conf can be the basis of values


// --- Main -----------------------------------------------------------------------------
void game_init(void); // user provided
void game_frame(void); // user provided

// --- Audio ----------------------------------------------------------------------------
// also see audio defines in kconf.h
void audio_init();
// user provided : fill a buffer with 8bit L/R sound data
void game_snd_buffer(uint16_t *buffer, int len);

// --- VGA interface ----------------------------------------------------------------------
// also check kconf.h for video modes.

// micro interface to the kernel (8bpp, mono sound). can be used on bitbox _board_ also.
#if VGA_BPP==8
	#define RGB(r,g,b)  (((r)&0xe0) | ((g)&0xc0)>>3 | (((b)&0xe0)>>5))
	typedef uint8_t pixel_t;
#else
	#define RGB(r,g,b)  ((((r)>>3)&0x1f)<<10 | (((g)>>3)&0x1f)<<5 | (((b)>>3)&0x1f))
	typedef uint16_t pixel_t;
#endif 

extern uint32_t vga_line; // should be const
extern volatile uint32_t vga_frame;

// in a physical line (on screen) but not a buffer refresh line (only used in half-height modes)
#ifdef VGA_SKIPLINE
extern volatile int vga_odd; 
#else
#define vga_odd 0 // never
#endif

extern void graph_line(void); // user provided graphical callback
extern void graph_vsync(void); // user provided, called during vsync lines

// 0x0rrrrrgggggbbbbb pixels or 0xrrrggbbl 
extern pixel_t *draw_buffer;  // drawing next line, 8 or 16bpp


// --- SD reader -------------------------------------------------------------------------
// use fatfs/ff.h api directly, separately.

void bitbox_init(); // init everything.

// --- user LED, user button -----------------------------------------------------------------
void set_led(int);
int button_state();

// --- input devices -------------------------------------------------------------------------

// structs and enums
enum device_enum {
	device_unconnected,
	device_keyboard,
	device_mouse,
	device_gamepad
};

enum gamepad_buttons_enum {
    gamepad_A = 1<<0,
    gamepad_B = 1<<1,
    gamepad_X = 1<<2,
    gamepad_Y = 1<<3,
    gamepad_L = 1<<4,
    gamepad_R = 1<<5,
    gamepad_select = 1<<6,
    gamepad_start = 1<<7,

    gamepad_up = 1<<8,
    gamepad_down =1<<9,
    gamepad_left = 1<<10,
    gamepad_right=1<<11,

	mousebut_left=gamepad_A,
	mousebut_right=gamepad_X,
	mousebut_middle=gamepad_B
};

#define GAMEPAD_PRESSED(id , key) (gamepad_buttons[id] & (gamepad_##key))
// -- state. defined in hid_gamepad.c
extern volatile enum device_enum device_type[2]; // currently plugged device

extern volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values
extern volatile uint16_t gamepad_buttons[2]; // simple mapping : ABXY LR Start Select UDLR xxxx

extern volatile int8_t mouse_x, mouse_y; // delta X,Y for this frame
extern volatile uint8_t mouse_buttons;

// Keyboard status : currently pressed keys
// also updates gamepad_buttons : space->select enter->start, udlr->arrows, keys->buttons : DFER -> ABXY, LR -> LR Ctrl.
extern volatile uint8_t keyboard_mod[2]; // LCtrl =1, LShift=2, LAlt=4, LWin - Rctrl, ...
extern volatile uint8_t keyboard_key[6][2]; // using raw USB key codes

// --- misc
void die(int where, int cause); // blink leds

// do nothing on device, printf it on emulator. redefined by serial to output to serial
void message (const char *fmt, ...);


