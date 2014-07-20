#pragma once

#include <stdint.h>
#include <bitbox.h>

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
object * btc4_2x_new (const uint32_t *btc, int16_t x, int16_t y, int16_t z) __attribute__ ((warn_unused_result)); 

object * tilemap_new (const uint16_t *tileset, int w, int h, uint32_t *data) __attribute__ ((warn_unused_result)); ;
/* 
	- tileset is a list of 16x16 u16 pixels  

	- width and height are displayed sizes, can be bigger/smaller than tilemap, in which case it will loop
	- tilemap is stored in data
	tilemap size can be 32x32, 64x32, 64x32, ... 
	tilemap references can be u16, i16 (semi transparent tiles), u8 or i8

	data : 
	    first u32 : header
	    	r....rssstt
	        r : reserved
	        sss : tilemap_size: 000: 64x64, ...
	        tt : tilemap_index_type = 00:u16, 01:u8, 10:i16, 11:i8
	    next words : tile_index either u8 or u16 ... 

*/

// tilemap : tilemap (64x64 u16= 1b semitrans + 3b tilemap + 12b index) + u16 *tilesets[8], + roll 64x64
