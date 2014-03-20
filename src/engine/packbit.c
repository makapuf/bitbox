
// TODO : handle 4bpp images

// -- packbit lines from palette sprite ------------------------------------------------------------------
// data from couples ?

// tester avec une valeur entree a la main

#include <stdint.h>
#include "blit.h"

// no need to include headers for that
void pre_draw (object *o, int16_t x1, int16_t x2,char is_opaque) ;

// from the base of the record
#define PB_HEADER_SIZE 2
static inline int pb_eol(uint8_t *pb_record)  { return *pb_record&1;}
static inline int pb_skip(uint8_t *pb_record) { return *pb_record>>1;}
static inline int pb_fill(uint8_t *pb_record) { return (*(pb_record+1))&0x80;}
static inline int pb_len(uint8_t *pb_record)  { return (*(pb_record+1))&0x7f;}

void packbit_frame(object *o, int start_line);
void packbit_line(object *o);
void packbit_blit(object *o, int16_t x1, int16_t x2);

int packbit_new (object *o, int16_t x, int16_t y, int16_t z, int line_ofs, const packbit_rom *pb)
{
    // packbit
    int i = blitter_insert((object *)o);    
    if (i<0) return i;

    // generic attributes x,y,z h,w
    o->x=x; o->y=y; o->z=z; 
    o->h=pb->h; o->w=pb->w;
    // methods
    o->frame=packbit_frame;
    o->line=packbit_line;
    o->blit=packbit_blit;

    // extra inter frame data
    o->a = (intptr_t) pb; 
    o->b = line_ofs;
    return i;
}


void packbit_frame(object *o, int start_line)
{
    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines

    start_line += PB_OFS(o); // add line offset (frame, ..) to start line

    uint8_t *p;
    // rewind pointer to nearest line start

    PB_PTR(o)=((intptr_t)PB_DATA(o)) + PB_IDX16(o)[start_line/16]; 
    start_line %= 16; // remainder
    while (start_line) {
        p = (uint8_t *)PB_PTR(o); // pot unaligned.
        if (pb_eol(p)) start_line--;
        // advances pointer to next blit anyway
        PB_PTR(o) += 2;
        PB_PTR(o) += pb_fill(p) ? 1 : pb_len(p); 
    }  
    //printf("now at %d\n",(uint8_t *)PB_PTR(o)-PB_DATA(o));
}

int ox; // XXX saves from line to 
void packbit_line(object *o)
{
    int x=o->x; // copy to the next 
    uint8_t *p;
    do {
        p = (uint8_t *)PB_PTR(o);
        //printf("len %d fi %d eol %d skip %d\n",pb_len(p),pb_fill(p),pb_eol(p),pb_skip(p));
        x += pb_skip(p);
        ox=x; // temp:x at start of blit after skip

        if (pb_len(p)) pre_draw (o, x,x+pb_len(p),OPAQUE); // will blit from ptr, maybe cut & restricted, maybe several times

        x += pb_len(p);
        // advances pointer to next blit
        PB_PTR(o) += 2; //sizeof(packbit_record);
        PB_PTR(o) += pb_fill(p) ? 1 : pb_len(p); 

        // skip a line. gets to the next blit & check eol
    } while (!pb_eol(p)); // eol
}

void packbit_blit(object * restrict o, int16_t x1, int16_t x2)
{
    // blit from the current ptr, either by set or cpy
    // current ptr is set to the start of the current blit.
    uint16_t * restrict dst = &draw_buffer[x1];
    uint8_t * restrict p = (uint8_t*) PB_PTR(o); // points to a record

    // XXX 2 par 2 ? unroll
    if (pb_fill(p))
    {
        uint16_t col = PB_PAL(o)[p[PB_HEADER_SIZE]];
        //printf("packbit filling from %d to %d with %d ...\n",x1,x2,PB_PAL(o)[p[PB_HEADER_SIZE]]);
        for (int i=x2-x1;i>=0;--i)
            *dst++ = col;  
    } else {
        uint8_t * restrict src = p+PB_HEADER_SIZE+x1-ox; // ox = x at start of blit (global) 
        uint16_t * restrict pal = PB_PAL(o);
        // copy with palette XXX 2 par 2 ?
        //printf("copy from %d to %d with %d ...\n",x1,x2,src-p- PB_HEADER_SIZE);

        for (int i=x2-x1;i>=0;--i) 
            *dst++=pal[*src++];

    }   
}
