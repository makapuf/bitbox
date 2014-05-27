
// TODO : handle 8bpp uncompressed images, simplify in RLE / masked blits

// -- packbit lines from palette sprite ------------------------------------------------------------------
// data from couples ?


// --------- packbit structure (as header, use with blit.c)

typedef struct 
{
    uint16_t w,h; // height is the height of one frame. can be smaller than image height if several frames.
    uint16_t frame_h; // index to data offset for each 16 lines. length = h/16
    uint16_t idx_len; // number of elements in the 16 lines offset index.
    uint16_t data[]; // index 16 lines + [PB_H + (opt) fill color + data ... ] x N
} packbit_rom;

#define PB_H(fill,skip,copy,eol) (fill<<9|skip<<8|eol<<7|copy)

int packbit_new (object *o, int16_t x, int16_t y, int16_t z, int line_ofs, const packbit_rom *pb);
// ------------------------------------

#include <stdint.h>
#include <string.h> // memcpy
void *memcpy2 (void *dst, const void *src, size_t count);

#include "blit.h"


#define PB_IDX16(x) ((packbit_rom*)(x->a))->data // start of offsets of each 16lines
#define PB_DATA(x) (((packbit_rom*)(x->a))->data+((packbit_rom*)(x->a))->idx_len)   // start of blit data, as uint16_t*

#define PB_OFS(x) ((x)->b) // line offset (start blitting at line X )
#define PB_PTR(x) ((x)->c) // current blit data ptr


// no need to include headers for that
void pre_draw (object *o, int16_t x1, int16_t x2,char is_opaque);

// (fill<<9|skip<<8|eol<<7|copy)
static inline int pb_fill(uint16_t *pb_hdr)  { return *pb_hdr>>9;}
static inline int pb_skip(uint16_t *pb_hdr) { return *pb_hdr&(1<<8);}
static inline int pb_eol (uint16_t *pb_hdr) { return *pb_hdr&0x80;}
static inline int pb_copy (uint16_t *pb_hdr)  { return *pb_hdr&0x7f;}

void packbit_frame(object *o, int start_line);
void packbit_line(object *o);
void packbit_blit(object *o, int16_t x1, int16_t x2);
void packbit_line_b(object *o); // no complex blits

int packbit_new (object *o, int16_t x, int16_t y, int16_t z, int line_ofs,const packbit_rom *pb)
{
    // packbit
    int i = blitter_insert((object *)o);    
    if (i<0) return i;

    // blitter object attributes x,y,z h,w
    o->x=x; o->y=y; o->z=z; 
    o->h=pb->frame_h; o->w=pb->w;

    // methods
    o->frame=packbit_frame;
    o->line=packbit_line_b;
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

    start_line += PB_OFS(o); // add line offset (used for multiframe sprites ...) to start line

    uint16_t *p;

    // ffwd pointer to nearest 16 line start
    PB_PTR(o)=((intptr_t)PB_DATA(o))+2*PB_IDX16(o)[start_line/16]; // + is integer, so byte offset here


    start_line %= 16; // remainder
    while (start_line) {
        p = (uint16_t *)PB_PTR(o); 
        if (pb_eol(p)) start_line--;
        // advances pointer to next blit anyway (as integer, so byte offsets)
        PB_PTR(o) += 2; // header
        PB_PTR(o) += 2*pb_copy(p);
        if (!pb_skip(p)) PB_PTR(o) += 2; 
    }  
}

uint16_t *blit_ofs; // blit pointer at start of blit copy data, corrected by blit start wrt line. zero if blit fill 

void packbit_line(object *o)
{
    int x=o->x; // copy to the next 
    uint16_t *p;
    do {
        p = (uint16_t *)PB_PTR(o); // (shortcut) start of the blit

        // fill blit
        if (!pb_skip(p) && pb_fill(p)) 
        {
            blit_ofs = 0; // do fill
            pre_draw (o, x,x+pb_fill(p),OPAQUE); 
        }
        x += pb_fill(p); // advance x
            
        // now, copy blit
        blit_ofs = p+1; // base blit + header
        if (!pb_skip(p)) blit_ofs += 1; // fill color : 1 uint16
        blit_ofs -= x; // corrected by starting x 
        pre_draw (o, x,x+pb_copy(p),OPAQUE);

        x += pb_copy(p);

        // advances pointer to next blit
        PB_PTR(o) += 2; // header130
        PB_PTR(o) += 2*pb_copy(p);
        if (!pb_skip(p)) PB_PTR(o) += 2; // color, byte offset here

        // skip a line. gets to the next blit & check eol
    } while (!pb_eol(p)); // stop if eol
}

void packbit_blit(object * restrict o, int16_t x1, int16_t x2) 
{
    // blit from the current ptr, either by set or cpy
    // current ptr is set to the start of the current blit.

    uint16_t * restrict p = (uint16_t*) PB_PTR(o); // points to record start

    if (!blit_ofs) // a fill (real)
    {
        fast_fill(x1,x2,p[1]);
    } else { // copy from x1 to x2
        //printf("copy from %d to %d with %d ...\n",x1,x2,src-p- PB_HEADER_SIZE);
        uint16_t * restrict dst = &draw_buffer[x1];
        uint16_t * restrict src = blit_ofs+x1;
        // XXX fast ASM memcpy ?
        memcpy2(dst, src,2*(x2-x1));
    }   
}


void packbit_line_b(object *o)
{
    int x=o->x; // copy to the next 
    if (x<0) x=0;

    uint16_t *p;
    do {
        p = (uint16_t *)PB_PTR(o); // (shortcut) start of the blit

        // fill blit
        if (!pb_skip(p) && pb_fill(p)) 
        {
            blit_ofs = 0; // do fill
            fast_fill(x,x+pb_fill(p),p[1]);
        }
        x += pb_fill(p); // advance x
        
        // now, copy blit
        blit_ofs = p+1; // base blit + header
        if (!pb_skip(p)) blit_ofs += 1; // fill color : 1 uint16
        memcpy2(&draw_buffer[x],blit_ofs,2*pb_copy(p));
        x += pb_copy(p);

        // advances pointer to next blit
        PB_PTR(o) += 2; // header130
        PB_PTR(o) += 2*pb_copy(p);
        if (!pb_skip(p)) PB_PTR(o) += 2; // color, byte offset here

        // skip a line. gets to the next blit & check eol
    } while (!pb_eol(p)); // stop if eol
}
