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

	// callbacks
	void (*frame)(struct object *o, int line); // prepare object ; not very clever
	void (*line)(struct object *o); 
	void (*blit)(struct object *o, int16_t x1,int16_t x2);

	void * static_data; // this will be the data stored in flash

	intptr_t a,b,c,d; // various 32b uses (make a real union ?)

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
int btc4_new(object *o, const uint32_t *btc, int16_t x, int16_t y, int16_t z); // btc4 palette


// useful  : fast fill with 16 bits pixels
void fast_fill(uint16_t x1, uint16_t x2, uint16_t c);


// --------- packbit structure

typedef struct 
{
    uint16_t w,h; // height is the height of one frame. can be smaller than image height if several frames.
    uint16_t frame_h; // index to data offset for each 16 lines. length = h/16
    uint16_t idx_len; // number of elements in the 16 lines offset index.
    uint16_t data[]; // index 16 lines + [PB_H + (opt) fill color + data ... ] x N
} packbit_rom;

#define PB_H(fill,skip,copy,eol) (fill<<9|skip<<8|eol<<7|copy)

int packbit_new (object *o, int16_t x, int16_t y, int16_t z, int line_ofs, const packbit_rom *pb);
