#pragma once

#include <stdint.h>
#include <kernel.h>

#define MAX_OBJECTS 64 // max objects present at the same time

typedef struct object
{
	// static data (typically in flash)
	void *data; // this will be the source data
	void (*frame)(struct object *o, int line);
	void (*line) (struct object *o); 

	// live data (typically in RAM, stable per frame)
	int32_t x,y,z;
	uint32_t w,h,fr; // h is one frame height, not full height, frame is frame id
	intptr_t a,b,c,d; // various 32b used for each blitter as extra parameters or internally

	// inline single linked lists (engine internal)
	struct object *activelist_next;
} object;  


void fast_fill(uint16_t x1, uint16_t x2, uint16_t c); // utility: fast fill with 16 bits pixels

void blitter_init();

// adds a new object in the , returns a handler. never insert an object twice 
int blitter_insert(object *o);
void blitter_remove(object *o);
// don't modify an object or add objects during a frame rendering. 

void blitter_frame(); // callback for frames.
void blitter_line();


object * rect_new(int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color) __attribute__ ((warn_unused_result));
object * sprite_new(uint32_t *sprite_data)  __attribute__ ((warn_unused_result));
object * btc4_new (const uint32_t *btc, int16_t x, int16_t y, int16_t z) __attribute__ ((warn_unused_result)); 

