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




// keymap : list of ranges of elements, non shifted and shifted
// see http://www.usb.org/developers/devclass_docs/Hut1_12.pdf, page 7
#define ERR KEY_ERR
static const char keyb_en[3][83] = { // normal, shift, ctrl
    {
        ERR,ERR,ERR,ERR,'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
        'o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6',
        '7','8','9','0','\n',0x1B,'\b','\t',' ','-','=','[',']','\\','#',';','\'','`',
        ',','.','/',
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP
    },{
        ERR,ERR,ERR,ERR,'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
        'O','P','Q','R','S','T','U','V','W','X','Y','Z','!','@','#','$','%','^',
        '&','*','(',')','\n',0x1B,'\b','\t',' ','_','+','{','}','|','#',':','"','~',
        '<','>','?',
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP,
    },{
        ERR,ERR,ERR,ERR, 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,'1','2','3','4','5','6',
        '7','8','9','0','\n',0x1B,'\b','\t',' ','-','=','[',']','\\',ERR,';','\'','`',
        ',','.','/',
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP,
    }
};

static const char keyb_fr[3][83] = { // normal, shift, ctrl - TODO : add right_alt characters (for ~ [], {}, # and @)
    {
        ERR,ERR,ERR,ERR,'q','b','c','d','e','f','g','h','i','j','k','l',',','n',
        'o','p','a','r','s','t','u','v','z','x','y','w','&',0xe9,'\"','\'','(','-',
        0xe8,'_',0xe7,0xe0,'\n',0x1B,'\b','\t',' ',')','=','^','$','\\','#','m',0xF9,'*',
        ';',':','!',
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP
    },{
        ERR,ERR,ERR,ERR,'Q','B','C','D','E','F','G','H','I','J','K','L','?','N',
        'O','P','A','R','S','T','U','V','Z','X','Y','W','1','2','3','4','5','6',
        '7','8','9','0','\n',0x1B,'\b','\t',' ',0xB0,'+',0xa8,0xa3,'\\','#','M','%',0xB5,
        '.','/',0xA7,
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP,
    },{
        ERR,ERR,ERR,ERR, 17, 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12,',', 14,
         15, 16, 1 , 18, 19, 20, 21, 22, 26, 24, 25, 23,'1','2','3','4','5','6',
        '7','8','9','0','\n',0x1B,'\b','\t',' ','-','=','[',']','\\',ERR,';','\'','`',
         13,'.','/',
        [79]=KEY_RIGHT,KEY_LEFT,KEY_DOWN,KEY_UP,
    }
}; 

// FIXME Put config in flash ?
#ifdef KEYB_FR
static const char (*keymap)[83]=keyb_fr;  
#else
static const char (*keymap)[83]=keyb_en; 
#endif 

// not public, not static either as it can be used by other funcs
char kbd_map(uint8_t mod, uint8_t key) 
{
    if (key>82) 
        return ERR;
    if (mod & (LShift|RShift))
        return keymap[1][key];
    if (mod & (LCtrl|RCtrl))
        return keymap[2][key];
    return keymap[0][key];
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



