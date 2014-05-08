// new lowlevel kernel base API

// to be provided by games or engines
#include <stdint.h>

void game_init();
void game_frame();
void game_line();
void game_audio_cb(uint16_t *buffer, int len); // user-provided

// --- Audio ----------------------------------------------------------------------------

int audio_on;

// --- VGA interface ----------------------------------------------------------------------

extern const int vga_line; 
extern const int vga_frame; 

extern int vga_pixel_clock; // linked to x resolution. max drawn pixels is 1024 : less means smaller pixels H

// 0x0rrrrrgggggbbbbb pixels
extern uint16_t draw_buffer[]; // drawing next line 


// --- SD reader -------------------------------------------------------------------------
// use ff.h api directly, separately.

// --- UEXT
// use SPI / I2C / Serial directly ? by device drivers

void bitbox_init(); // init everything.

// --- user LED, user button -----------------------------------------------------------------
void set_user_led(int);
int user_button_state(); // in an event ?

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
	// Those values are duplicates four mouse. 
	but_left=but_A,but_right=but_B,but_middle=but_X 
};


enum evt_type { // type (a,b,c data)
	no_event=0x00, // (error / empty queue)
	evt_device_plugged, // port, device type
	evt_device_unplugged, // port, device type

	evt_mouse_move,   // (port, x, y) - relative x,y !
	evt_mouse_click,  // (port, button_id)
	evt_mouse_release,// (port, button id)

	evt_gamepad_press, // (port, button id)
	evt_gamepad_release, // (port, button id)
	evt_gamepad_axis, // change (port, x,y) 

	evt_keyboard_press,  // (u8 key, u8 modifiers)
	evt_keyboard_release,// (u8 key, u8 modifiers)

	evt_user_button, // none


	evt_user, 
/*
 (other events sent by other engines)
 timer finished   (timed id ?)

 SD data ready  - async read finished (bof, < frame en general ?)
 SD card inserted 
 SD Card withdrawn
 
 user button press / release (if interrupt based ?)
 user button release

 UART data available
 SPI DMA ready ?

 Sprite Collisions 

*/
} ;

enum kbd_modifier { 
	LCtrl,LShift,LAlt,LWin,RCtrl,RShift,RAlt,RWin 
} ;


struct event {
	uint8_t type;
	union {
		struct {
			uint8_t port;
			uint8_t id;
		} button;

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

// status
extern device_enum device_type[2]; // currently plugged device
extern int16_t gamepad_x[2], gamepad_y[2], gamepad_buttons[2]; // simple mapping


// --- event functions 
void event_clear();

// ignores content if try to insert on a full queue 
void event_push(event e);

// returns "empty event"=0 if get from empty
event event_get();

char kbd_map(event kbd_e);