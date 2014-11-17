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
	int32_t x,y,z,ry; // ry is real Y, while y is wanted y, which will be activated next frame.
	uint32_t w,h,fr; // h is one frame height, not full height, frame is frame id
	intptr_t a,b,c,d; // various 32b used for each blitter as extra parameters or internally

	// inline single linked lists (engine internal)
	struct object *activelist_next;
} object;  


void fast_fill(uint16_t x1, uint16_t x2, uint16_t c); // utility: fast fill with 16 bits pixels

void blitter_init(void);
object *blitter_new(void);
void blitter_remove(object *o);
// don't modify an object or add objects during a frame rendering. 

void blitter_frame(void); // callback for frames.
void blitter_line(void);

// creates a new object, activate it, copy from object.
object *object_new(const object *from) __attribute__ ((warn_unused_result));

object * rect_new(int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color) __attribute__ ((warn_unused_result));
object * sprite_new(uint32_t *sprite_data)  __attribute__ ((warn_unused_result)); // XXX passer en void*
object * btc4_new (const uint32_t *btc, int16_t x, int16_t y, int16_t z) __attribute__ ((warn_unused_result)); 
object * btc4_2x_new (const uint32_t *btc, int16_t x, int16_t y, int16_t z) __attribute__ ((warn_unused_result)); 

#define TSET_16 0
#define TSET_32 1
#define TMAP_U8 1

#define TMAP_HEADER(w,h,tilesize,tmaptype) (w<<24|h<<16|tilesize<<8|tmaptype)

#define TILEMAP_6464u8 64<<24|64<<16|TSET_16<<8|TMAP_U8
#define TILEMAP_3232u8 32<<24|32<<16|TSET_16<<8|TMAP_U8
#define TILEMAP_6464u832 32<<24|32<<16|TSET_32<<8|TMAP_U8

object * tilemap_new (const uint16_t *tileset, int w, int h, uint32_t header, void *tilemap) __attribute__ ((warn_unused_result));
/* 
	- tileset is a list of 16x16 u16 pixels. It will be 1-indexed by tilemap.
    - width and height are displayed sizes, can be bigger/smaller than tilemap, in which case it will loop
	tilemap size can be 32x32, 64x32, 64x32, (or any other, but need to be standard)
	tilemap references can be u16, i16 (semi transparent tiles), u8 or i8 (not all implemented now)

	header
	        u8 : width of tilemap in tiles
	        u8 : height in tiles
	        u8 : 2 low bits : tilesize. 00 = 16x16, 01 = 32x32 tiles
	        u8 : tilemap_index_type = 0:u16, 1:u8, 2:i16, 3:i8
	void * data : tile_index either u8 or u16 ... 

	tilemap index 0 are always transparent (ie no tile, so first tile in tileset has index 1)
*/

void tmap_blit(object *dst, int x, int y, uint32_t src_header, const void *src_data);
/* 
	blits a tilemap on the object tilemap at x,y position (x,y in tiles)
 */
