/*
 * Bitbox event handling library. This is useful to generate events and better keybpard handling.
 * based on bitbox state globals, you will need to call events_poller each frame.  
 */

#include "bitbox.h"

// Call this every frame to check what changed and emit events
void events_poll();

enum keycodes {
	KEY_ERR = 0xff,
	KEY_RIGHT = 128,
	KEY_LEFT = 129,
	KEY_DOWN = 130,
	KEY_UP = 131
};

enum kbd_modifier {
	LCtrl = 1,
	LShift = 2,
	LAlt = 4,
	LMeta = 8,
	
	RCtrl = 16,
	RShift = 32,
	RAlt = 64,
	RMeta = 128
};


enum evt_type { // type (a,b,c data)
	no_event=0x00, // (error / empty queue)
	evt_device_change, // device : port, device type, subtype. Set device type to 0 to mean unconnect

	evt_mouse_move,   // (port, x, y) - relative x,y !
	evt_mouse_click,  // button : (port, button_id)
	evt_mouse_release,// button : (port, button id)

	evt_keyboard_press,  // kbd (u8 key, u8 modifiers : R Gui/Alt/Shift/Ctrl, L ...
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

struct event { //should be coded in 32bits
	uint8_t type;
	union {
		struct {
			uint8_t port; // 0 or 1
			uint8_t type; // type of device
			uint8_t subtype; // subtype (if any)
		} device;

		struct {
			uint8_t port; // USB port 0 or 1
			uint8_t id; // id of button pressed
		} button;

		// used for mice
		struct {
			uint8_t port;
			int8_t x,y;
		} mov;

		struct {
			uint8_t key; // key is the boot protocol physical key pressed,
			uint8_t mod; // modifier bitmask : LCtrl, LAlt, ...
			uint8_t sym; // symbol is the ascii code or logical key pressed (including KEY_RIGHT as defined )
		} kbd;

		uint8_t data[3]; // raw value
	};
} __attribute__ ((__packed__));

void event_clear();

// ignores content if try to insert on a full queue
void event_push(struct event e);

// returns "empty event"=0 if get from empty
struct event event_get();

