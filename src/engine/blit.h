#pragma once

#include <stdint.h>
#include <kernel.h>

#define MAX_BLITS 32 // max blits per line (note that blits can be coalesced by engine)
#define MAX_TRANSP 16 // max nb obj transparent objects
#define MAX_OBJECTS 32 // max objects present at the same time

#define OPAQUE 1
#define TRANSPARENT 0


typedef struct object
{
	int16_t x,y,z,w,h; // keep w for collision

	// callbacks - make a vtable ?
	void (*frame)(struct object *o, int line);
	void (*line)(struct object *o); // calls make_opaque, which will call blit
	void (*blit)(struct object *o, int16_t x1,int16_t x2);

	intptr_t a,b,c,d; // various 32b uses 

	// inline single linked lists
	struct object *activelist_next;
} object;  


void blitter_init();

// adds a new object, returns a handler
int blitter_insert(object *o);
void blitter_remove(int i);
// don't modify an object or add objects during a frame rendering. don't do that.

void blitter_frame(); // callback for frames.
void blitter_line();

int new_opaquerect (object *o, int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color);
int new_checkerrect(object *o, int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color1, uint16_t color2);

// --------- packbit structure

typedef struct 
{
    uint16_t w,h; // height is the height of one frame. can be smaller than image height if several frames.
    uint16_t *palette; // u16 color used by indices.
    uint16_t const *idx16; // index to data offset for each 16 lines. length = h/16
    uint8_t const *data; // header+ data bytes + header ...
} packbit_rom;

#define PB_DATA(x) ((packbit_rom*)(x->a))->data   // start of blit data
#define PB_PAL(x) ((packbit_rom*)(x->a))->palette // start of palette
#define PB_IDX16(x) ((packbit_rom*)(x->a))->idx16 // start of offsets of each 16lines

#define PB_OFS(x) ((x)->b) // line offset (start blitting at line X )
#define PB_PTR(x) ((x)->c) // current blit data ptr
int packbit_new (object *o, int16_t x, int16_t y, int16_t z, int line_ofs, const packbit_rom *pb);

