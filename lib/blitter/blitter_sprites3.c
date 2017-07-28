// blitter_sprites3.c : simpler blitter objects. .
// All in 8bpp, define vga_palette symbol as uint16_t [256] to expand later

/* object layout in memory : 
 data : raw data (start of blits)
 a : line16 skips
 b,d :  not used
 c : current blitting pos in data
 */

#include <string.h> // memcpy, memset
#include "blitter.h"

static void sprite3_frame(struct object *o, int line);
static void sprite3_line_noclip(struct object *o);
//static void sprite3_line_clip(struct object *o);

/* 	data format : 
	width, height, nb_frames, _RFU_ : u16
	line16 : height/16 hwords (16) with absolute offset from absolute start of file to start of line
*/
struct object *sprite3_new(const void *p, int x, int y, int z)
{
	object *o = blitter_new();
    if (!o) return 0; // error : no room left

    uint16_t *header=(uint16_t*)p;
    if (header[0]!=0xB17B && header[0]!=0x1808) {
    	message("Error : wrong header found");
    	die(8,8);
    }

    o->w = header[2];
    o->h = header[3];

    o->a = (uintptr_t)header+6;
    o->data = header+6+(o->h/16);

    o->frame = sprite3_frame;

    o->x=x;
    o->y=y;
    o->z=z;
    o->fr=0;

    return o;
}

static void sprite3_frame(struct object *o, int start_line)
{

    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines
    //  nb skip:4, nb blit: 3, eol:1
    // a: line16, c: current data

    start_line += o->fr*o->h;
    o->c = (intptr_t)o->data;

    // Skip first lines as needed
    o->c += ((uint16_t*)o->a)[start_line/16]; // skip bytes
    start_line %= 16; // remainder

    int pixels=start_line*o->w; // remaining pixels to skip
    while (pixels>0) {
        uint8_t h = *(uint8_t *)o->c++; // blit header
        pixels -= h>>2;
        switch(h&3) {
        	case 0 : break; // skip
        	case 1 : o->c += h>>2; // literal
        	case 2 : o->c += 2; // backref 
        	case 3 : o->c += 1; // fill
        }
    }

    /*
    // select if clip or noclip
    if ( o->x < -16 || o->x - o->w >= VGA_H_PIXELS+16 )
        o->line = sprite3_line_clip;
    else 
	*/
    o->line = sprite3_line_noclip;
}

static void sprite3_line_noclip (struct object *o)
{
    int x=o->x;
    uint8_t *src = (uint8_t *)o->c; // see blit as bytes. p is always the start of the blit

    while(x<o->x+o->w) {
        uint8_t header = *src++; // (shortcut) start/current position of the blit
        uint8_t nb_pix = header>>2;
        switch (header & 3) {
        	case 0 : // skip
        		break;
        	case 1 : // literal
        		memcpy(&draw_buffer[x],src,nb_pix);
        		src+=nb_pix;
        		break;
        	case 2 : // back reference
        		memcpy(&draw_buffer[x],o->data + ((src[0]<<8) + src[1]),nb_pix);
        		src+=2;
        		break;
        	case 3 : 
        		memset(&draw_buffer[x],src[0],nb_pix);
        		src +=1;
        		break;
        }
        x += nb_pix; // in any case we processed nbpix pixels
    }
    o->c = (uintptr_t) src;
}


