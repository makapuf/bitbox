#pragma once

#include <stdint.h>

#define MAX_BLITS 32 // max blits per line
#define MAX_TRANSP 16 // max nb obj transparent objects
#define MAX_OBJECTS 32 // max objects present at the same time

#define MAXINT 32767
#define MININT -32767
#define SCREEN_WIDTH 640
#define OPAQUE 1
#define TRANSPARENT 0


typedef struct object
{
	int16_t x,y,z,w,h; // keep w for collision

	// callbacks - make a vtable ?
	void (*frame)(struct object *o, int line);
	void (*line)(struct object *o); // calls make_opaque, which will call blit
	void (*blit)(struct object *o, int16_t x1,int16_t x2);

	int a,b,c; // various 32b uses 

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


/*
 * declare opaque for this line . predraw will call the blitting on this object as necessary
 */	
void pre_draw (object *o, int16_t x1, int16_t x2,char is_opaque) ;



void opaquerect_line (object *o);
void transprect_line (object *o);
void colorfill_blit(object *o, int16_t x1, int16_t x2);


