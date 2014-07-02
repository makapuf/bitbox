// new lowlevel kernel base API
#pragma once

// to be provided by games or engines
#include <stdint.h>

// user-provided
void game_init();
void game_frame();
void game_line();
void game_snd_buffer(uint16_t *buffer, int len); // **buffer plutot ?
// user provided : fill a buffer with 8bit L/R sound data

// --- Audio ----------------------------------------------------------------------------


#define BITBOX_SAMPLERATE 31469 
#define BITBOX_SNDBUF_LEN (BITBOX_SAMPLERATE/60) // 524

extern int audio_on; 

void audio_init();
void audio_frame(); // will call audio callback

// --- VGA interface ----------------------------------------------------------------------
#define RGB(r,g,b)  (((r>>3)&0x1f)<<10 | ((g>>3)&0x1f)<<5 | ((b>>3)&0x1f))

#define LINE_LENGTH 640 
extern uint32_t vga_line; // should be const
extern volatile uint32_t vga_frame; 

//extern int vga_pixel_clock; // linked to x resolution. max drawn pixels is 1024 : less means smaller pixels H
// all reglages..

// 0x0rrrrrgggggbbbbb pixels
extern uint16_t *draw_buffer; // drawing next line 


// --- SD reader -------------------------------------------------------------------------
// use ff.h api directly, separately.

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

enum buttons_enum { 
	but_A=1,but_B,but_X,but_Y,but_L,but_R,but_select,but_start,
	// Those values are alternate names for mouse. 
	but_left=but_A,but_right=but_B,but_middle=but_X 
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
    gamepad_right=1<<12,
};

#define GAMEPAD_PRESSED(id , key) (gamepad_buttons[id] & (gamepad_##key))

struct event {
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
} ;

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


// misc
void die(int where, int cause); // blink leds 
void message(char *msg); // do nothing on device, printf it on emulator