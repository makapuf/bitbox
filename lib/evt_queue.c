#include "bitbox.h"

// - device handling -------------------------------------------------------------------------------------------------------------------------------------


#define EVT_QUEUE_SIZE 64


volatile enum device_enum device_type[2]; // currently plugged device

/* Circular FIFO. 

- ignores content if try to insert on a full queue 
- returns "empty event"=0 if get from empty

 ---------xxxxxxx--------
          ^      ^
          out    in

*/

#define QUEUE_START (&evt_queue[0])          
#define QUEUE_END (&evt_queue[EVT_QUEUE_SIZE])          


//- event queue implementation ---------------------------------------------------------------------------------------------------------


static struct event evt_queue[EVT_QUEUE_SIZE];
static struct event *evt_in=QUEUE_START, *evt_out=QUEUE_START; 
// queue is empty if in=out; full if just before end (keep at least one empty)
// in : next to write, out = next place to write

static inline int event_full()
{
	return (evt_in+1==evt_out || (evt_in==QUEUE_END && evt_out==QUEUE_START));
}

static inline int event_empty() 
{
	return evt_in==evt_out; 
}

void event_push(struct event e)
{
	// full ? don't push
	if (event_full()) return; 
	*evt_in++ = e;
	// end of line ? rewind
	if (evt_in==&evt_queue[EVT_QUEUE_SIZE]) {
		evt_in=&evt_queue[0];
	}
}

struct event event_get()
{
	struct event e;
	// empty ? return empty event
	if (event_empty()) return (struct event){.type=no_event};
	e=*evt_out++;
	if (evt_out==&evt_queue[EVT_QUEUE_SIZE]) {
		evt_out=&evt_queue[0];
	}
	return e;
}

void event_clear()
{
	evt_in=evt_out=&evt_queue[0];
}


// keymap : list of ranges of elements, non shifted and shifted
// see http://www.usb.org/developers/devclass_docs/Hut1_11.pdf, page 7
const char keyb_en[56]    = "abcdefghijklmnopqrstuvwxyz1234567890\n\027\b\t -=[]\\#;'`,./";
const char keyb_en_sh[56] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%%^*()\n\027\b\t _+{}|#:\"~<>?" ;
const char keyb_fr[60]    = "qbcdefghijkl,noparstuvzxyw&é\"'(-è_çà\n\027\b\t )=^$\\#mù*;:!";
const char keyb_fr_sh[60] = "QBCDEFGHIJKL?NOPARSTUVZXYW1234567890\n\027\b\t °+¨£\\#M%%µ./§";

const char *keymap=keyb_en, *keymap_sh=keyb_en_sh; // XXX config ?

char kbd_map(struct event e) 
/*
 maps a keyboard event to printable latin-1 letter given the current keymap & shift. 
 only printable output is given, zero else.
 */
{
	if (e.kbd.key>=3 && e.kbd.key<=58) 
		return (e.kbd.mod & (LShift|RShift)) ? keymap_sh[e.kbd.key-3] : keymap[e.kbd.key-3];
	else 
		return 0; // non printable char
}


/* This emulates the gamepad with a keyboard.
 * fetches all keyboard events, 
 * discarding all others (not optimal)
 * mapping: 

    Space : Select,   2C
    Enter : Start,    28
    UDLR arrows : D-pad    52, 51, 50, 4F
    D     : A button, 07
    F : B button, 09
    E : X button, 08
    R : Y button, 15
    Left/Right CTRL (L/R shoulders) 
 */

void kbd_emulate_gamepad (void)
{
	// kbd codes in order of gamepad buttons
	static const uint8_t kbd_gamepad[] = {0x07, 0x09, 0x08, 0x15, 0xE0, 0xE4, 0x2c, 0x28, 0x52, 0x51, 0x50, 0x4f }; 
	// 
	struct event e;
	do {
		e=event_get();
		for (int i=0;i<12;i++) 
		{
			if (kbd_gamepad[i]==e.kbd.key) 
			switch (e.type)
			{
				case evt_keyboard_press  :
					gamepad_buttons[0] |= (1<<i);
				break;
			
				case evt_keyboard_release : 
					gamepad_buttons[0] &= ~(1<<i);
				break;
			}
		}
	} while (e.type);
}

#ifdef TEST
#include <stdio.h>

static void event_test()
{
	// test vectors
	const int add[] = {0,2,3,10,6 ,4,0,16,0 ,16,16,16,16,16,16,0};
	const int get[] = {2,0,6,0 ,16,0,4,0 ,16,0,0,0,0,0,0,16};

	event_clear();
	for (int i=0;i<sizeof(add)/sizeof(int);i++)
	{
		printf("round %d:+%d-%d:",i,add[i],get[i]);
		for (int j=0;j<add[i];j++) event_push((event){.raw=i});
		for (int j=0;j<get[i];j++) printf("%2d ",event_get(i));
		//printf("in: %p out:%p start:%p end:%p",evt_in, evt_out, QUEUE_START,QUEUE_END);
		if (event_full()) printf(" - now full ");
		if (event_empty()) printf("- now empty ");
		printf("\n");
	}
}

int main()
{
	printf("%d event, %d queue \n",sizeof(event),sizeof(evt_queue) );
	event_test();

	return 0;
}
#endif



