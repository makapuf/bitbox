// new lowlevel kernel base API

// to be provided by games or engines
void game_init();
void game_line();
void game_frame();

// --- Audio ----------------------------------------------------------------------------

void audio_init();
int audio_on;
void game_audio_cb(uint16_t *buffer; int len); // user-provided


// --- VGA -------------------------------------------------------------------------------

const int vga_line; 
const int vga_frame; 

int vga_pixel_clock; // linked to x resolution ; can set to . max drawn pixels is 1024 : less means smaller pixels H
int vga_pixel_drawn;

// 0x0rrrrrgggggbbbbb pixels
extern uint16_t draw_buffer[]; // drawing next line 


// --- SD reader -------------------------------------------------------------------------
// use ff.h api directly, separately.

// --- UEXT
// use SPI / I2C / Serial directly ? by device drivers

bitbox_init(); // init everything.

// --- user LED, user button -----------------------------------------------------------------
void set_user_led(int);
int user_button_state(); // in an event ?

// --- input devices -------------------------------------------------------------------------


// typedefs and enums
typedef enum { 
	device_unconnected,
	device_keyboard,
	device_mouse,
	device_gamepad 
} device_enum;

typedef enum { 
	but_A=1,but_B,but_X,but_Y,but_L,but_R,but_select,but_start,
	// Those values are duplicates four mouse. 
	but_left=but_A,but_right=but_B,but_middle=but_X 
} buttons_enum;


typedef enum { // type (a,b,c data)
	no_event=0x00, // (error)
	device_plugged,
	device_unplugged,

	mouse_move,   // (port, rel_x, rel_y) - relative x,y !
	mouse_click,  // (port, button_id)
	mouse_release,// (port, button id)

	gamepad_press,// (port, button id)
	gamepad_release,
	gamepad_axis,// change (port, x,y) 

	keyboard_press,  // (u8 key, u8 modifiers)
	keyboard_release,// (u8 key, u8 modifiers)

	user_event, 
/*
 (other events sent by other engines)
 timer finished   (timed id ?)

 SD data ready  - async read finished 
 SD card inserted 
 SD Card withdrawn
 
 user button press
 user button release

 UART data available

 Sprite Collisions 

*/
}  evt_type;

typedef enum { 
	LCtrl,LShift,LAlt,LWin,RCtrl,RShift,RAlt,RWin 
} kbd_modifier;

typedef struct {
	uint8_t type,a,b,c;
} event;


extern device_enum device_type[2]; // currently plugged device
extern int16_t x[2], y[2], buttons[2]; // simple mapping.

void event_clear();
void event_push(event e);
event event_get();
char kbd_map(event kbd_e);