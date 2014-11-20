// new lowlevel kernel base API
#pragma once

// to be provided by games or engines
#include <stdint.h>
#include "kconf.h" // kernel conf can be the basis of values

// user-provided
void game_init();
void game_frame();
void game_line();
void game_snd_buffer(uint16_t *buffer, int len); // **buffer plutot ?
// user provided : fill a buffer with 8bit L/R sound data

// --- Audio ----------------------------------------------------------------------------

#define BITBOX_SAMPLERATE VGA_VFREQ 
#define BITBOX_SNDBUF_LEN (BITBOX_SAMPLERATE/VGA_FPS+1) // 525
#define BITBOX_SAMPLE_BITDEPTH 8 // 8bit output 

extern int audio_on; 

void audio_init();


// --- VGA interface ----------------------------------------------------------------------
#define RGB(r,g,b)  (((r>>3)&0x1f)<<10 | ((g>>3)&0x1f)<<5 | ((b>>3)&0x1f))

extern uint32_t vga_line; // should be const
extern volatile uint32_t vga_frame; 

extern void graph_line(void); // user provided graphical 
extern void graph_frame(void); // user provided graphical blitting algorithms

// 0x0rrrrrgggggbbbbb pixels
extern uint16_t *draw_buffer; // drawing next line 
// also check kconf.h for video modes.


// --- SD reader -------------------------------------------------------------------------
// use fatfs/ff.h api directly, separately.

// --- UEXT
// use SPI / I2C / Serial directly ? by device drivers

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

enum evt_type { // type (a,b,c data)
	no_event=0x00, // (error / empty queue)
	evt_device_change, // device : port, device type, subtype. Set device type to 0 to mean unconnect

	evt_mouse_move,   // (port, x, y) - relative x,y !
	evt_mouse_click,  // button : (port, button_id)
	evt_mouse_release,// button : (port, button id)

	evt_keyboard_press,  // kbd (u8 key, u8 modifiers)
	evt_keyboard_release,// kbd (u8 key, u8 modifiers)

	evt_user, // sent by program
/*
 (other events sent by other engines)
 gamepad events ?

 timer finished   (timed id ?)

 SD card inserted 
 SD Card withdrawn
 SD data ready  - async read finished (difficult)
 
 user button press / release (if interrupt based ?)
 user button release

 UART data available
 SPI DMA ready ?

 Sprite Collisions 

*/
} ;

enum kbd_modifier { 
	LCtrl,LShift,LAlt,LWin,RCtrl,RShift,RAlt,RWin 
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
	mousebut_right=gamepad_B,
	mousebut_middle=gamepad_X 
};

#define GAMEPAD_PRESSED(id , key) (gamepad_buttons[id] & (gamepad_##key))

struct event { //should be coded in 32bits 
	uint8_t type;
	union {
		struct {
			uint8_t port;
			uint8_t type;
			uint8_t subtype;
		} device;

		struct {
			uint8_t port;
			uint8_t id;
		} button;
		
		// used for mice
		struct {
			uint8_t port;
			int8_t x,y;
		} mov;

		struct {
			uint8_t key, mod;
		} kbd;

		uint8_t data[3];
	};		
} __attribute__ ((__packed__));

// -- state. defined in usb_devices.c
extern volatile enum device_enum device_type[2]; // currently plugged device

extern volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values
extern volatile uint16_t gamepad_buttons[2]; // simple mapping : ABXY LR Start Select UDLR xxxx

extern volatile int data_mouse_x;
extern volatile int data_mouse_y;
extern volatile uint8_t data_mouse_buttons;
// extern volatile keyboard status (mod+6touches)

// --- event functions 
void event_clear();

// ignores content if try to insert on a full queue 
void event_push(struct event e);

// returns "empty event"=0 if get from empty
struct event event_get();

char kbd_map(struct event kbd_e);

/* This emulates the gamepad with a keyboard.
 * fetches all keyboard events, 
 * discarding all others (not optimal)
 * mapping: 

    Space : Select,   2C
    Enter : Start,    28
    UDLR arrows : D-pad    52, 51, 50, 4F
    D : A button, 07
    F : B button, 09
    E : X button, 08
    R : Y button, 15
    Left/Right CTRL (L/R shoulders) 
 */
void kbd_emulate_gamepad (void);

// --- misc
void die(int where, int cause); // blink leds 

// do nothing on device, printf it on emulator
// please only %s, %d, %x and %p, no format qualifiers
void message (const char *fmt, ...); 


