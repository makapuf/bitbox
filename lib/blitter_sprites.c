#include "blitter.h"

#include <string.h> // memcpy
// -------------------------------------------

/*

A sprite in memory is made of those records.
  Header : w,h,frames
  Line16 : u16 index_of_words starting each lines within data
  u32 data[] ((skip:7,misc:1, blit:7,eol:1, u16 extra) , u32[] )*

  skip&blit : pixel ou couples
  palette : u16[] pour p4, u32 pour c8

  p4 : u16 pixels palette  + u4 indices []
  p8 : idem, 8 bits
  rle : (skip, blit, u16 after : data)
  c8 : index in u32 couples palette
  u16 : direct blit of data


Object storage :
    data : pixel data
    a : palette (not used if u16) ; can be a palette of couples if c8
    b : line16 skips (skips as number of u32 words !)
    c : current blitting pos in data
    d : len per word (items per word, eg 8bits couples -> 4). 0 means not defined (eg RLE)

File storage of data : as record-based data. allows for RAM or flash usage.
    a record is made of
    - u32 type ID
    - u32 size in bytes of record, excluding. Size is always a 4 multiple
    - void* data[size]
*/


/*
 * Blit a line on the output. A line is a list (terminated by EOF), aligned on u32
 * data are u8 indices
 * transparency of pixels is not handled in the middle of the blit, only at the beginning/end.
 *
 * data format : always u32 aligned by blit (not lines)
 * blits : u16, p4, c8 ... -> different nb of pixels per word.
 * all blits are always skip:7, rfu:1, eol:1, len:7
 */
#include <stddef.h>

// this is internal for loading data
enum sprite_recordID {
    sprite_recordID__header=0,
    sprite_recordID__palette=1,
    sprite_recordID__line16=2,
    sprite_recordID__palette_couple=3,
    sprite_recordID__u16=1001,
    sprite_recordID__p4=1002,
    sprite_recordID__p8=1003,
    sprite_recordID__c8=1004,
    sprite_recordID__rle=1005,
    // 8bit
    sprite_recordID__u8=1006,

    sprite_recordID__end=32767,

};

#ifdef VGA_8BIT
static void sprite_frame8(object *o, int start_line);
#else
static void sprite_frame(object *o, int start_line);
#endif

static void sprite_u16_line(object *o);
static void sprite_p4_line(object *o);
static void sprite_c8_line(object *o);
static void sprite_rle_line(object *o);
static void sprite_u8_line(object *o);
static void sprite_p8_line(object *o) {}

#define EOL(x) (x&1)
#define LEN(x) ((x>>1) & 0x7f)
#define SKIP(x) ((x>>9) & 0x7f)
#define EXTRA(x) (x>>16)

object * sprite_new(const void *p, int x, int y, int z)
{
    uint32_t *sprite_data=(uint32_t*) p;

    object *o = blitter_new();
    if (!o) return 0; // error : no room left

    uint32_t t=-1, sz;

    #ifdef VGA_8BIT
    o->frame = sprite_frame8;
    #else
    o->frame = sprite_frame;
    #endif

    while (t!=sprite_recordID__end)
    {
        t=*sprite_data++;
        sz=*sprite_data++;
        switch(t)
        {
            case sprite_recordID__header :
                o->w = *sprite_data++;
                o->h = *sprite_data++;
                break;

            case sprite_recordID__palette :
                o->a = (uintptr_t)sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__palette_couple :
                o->a = (uintptr_t)sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__end :
                break;

            case sprite_recordID__line16 :
                o->b = (uintptr_t)sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__p8 :
                o->line = sprite_p8_line;
                o->data = sprite_data;
                o->d = 4;
                sprite_data += (sz+3)/4; // skip, don't read ; incl padded
                break;

            case sprite_recordID__c8 :
                o->line = sprite_c8_line;
                o->data = sprite_data;
                o->d = 4;
                sprite_data += (sz+3)/4; // skip, don't read ; incl padded
                break;

            case sprite_recordID__p4 :
                o->line = sprite_p4_line;
                o->data = sprite_data;
                o->d=8;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__u16 :
                o->line = sprite_u16_line;
                o->data = sprite_data;
                o->d=2;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__rle :
                o->line = sprite_rle_line;
                o->data = sprite_data;
                o->d=0;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__u8 :
                // check u8 ?
                o->line = sprite_u8_line;
                o->data = sprite_data;
                o->d=0;
                sprite_data += (sz+3)/4; // skip, don't read
                break;


            default :
                return 0; // error : unknown record !
                break;
        }
    }
    o->ry=0;
    o->c = (uintptr_t) o->data; // ry=0 !
    //
    o->x=x;
    o->y=y;
    o->z=z;
    o->fr=0;
    return o;
}

#ifndef VGA_8BIT
static void sprite_frame (object *o, int start_line)
{
    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines

    start_line += o->fr*o->h;

    o->c = (intptr_t)o->data + ((uint16_t*)o->b)[start_line/16]*4; // skip words=4bytes
    start_line %= 16; // remainder


    // o->d = lpw = len per word (items per word, eg 8bits couples -> 4) Not defined for RLE

    // Skip first lines as needed
    while (start_line) {
        uint32_t *p = (uint32_t *)o->c; // just a shortcut
        if EOL(*p) start_line--;

        // advances pointer to next blit anyway (as integer, so byte offsets)
        o->c += 4; // header word
        if (o->d)
            o->c += ((LEN(*p)+o->d-1-(o->d/2))/o->d)*4;
    }
}
#else
static void sprite_frame8 (object *o, int start_line)
{
    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines
    //  nb skip:4, nb blit: 3, eol:1

    start_line += o->fr*o->h;
    o->c = (intptr_t)o->data;

    o->c += ((uint16_t*)o->b)[start_line/16]; // skip bytes
    start_line %= 16; // remainder


    // Skip first lines as needed
    while (start_line) {
        uint8_t *p = (uint8_t *)o->c; // just a shortcut
        if (*p&1) start_line--;

        // advances pointer to next blit anyway (as integer, so byte offsets)
        o->c += 1; // header
        o->c += *p>>1 & 0x7; // data
    }
}
#endif

static void sprite_u16_line (object *o)
{
    int x=o->x;
    uint16_t *p; // see blit as half words. p is always the start of the blit
    do {
        p = (uint16_t *)o->c; // (shortcut) start/current position of the blit

        x += (*p)>>9; // skip : advance x

        // now, directly copy blit it
        uint32_t data_len = *p>>1 & 0x7f; // LEN but for 16bit

        memcpy(&draw_buffer[x], p+1, 2*data_len); // source = u16 after p

        // advance x
        x += data_len;

        // advances pointer to next blit
        o->c += 2; // data len
        o->c += 2*data_len;

        // skip a line. gets to the next blit & check eol
    } while (!(*p & 1)); // stop if it was an eol
}

static void sprite_u8_line (object *o)
{
    uint8_t *draw8 = (uint8_t*) draw_buffer; // draw buffer as bytes

    int x=o->x;
    uint8_t *p; // see blit as bytes. p is always the start of the blit
    do {
        p = (uint8_t *)o->c; // (shortcut) start/current position of the blit
        // header : nb skip:4, nb blit: 3, eol:1

        x += (*p)>>4; // skip : advance x

        // now, directly copy blit it
        uint32_t data_len = *p>>1 & 0x7; // LEN but for 8bit

        memcpy(&draw8[x], p+1, data_len);

        // advance x
        x += data_len;

        // advances pointer to next blit
        o->c += 1; // data len
        o->c += data_len;

        // skip a line. gets to the next blit & check eol
    } while (!(*p & 1)); // stop if it was an eol
}

static void sprite_rle_line (object *o)
{
    int x=o->x;
    uint32_t *p; // see blit as words. p is always the start of the current blit
    // data format : nb:13,2:RFU,1 EOL ; u16 color
    do {
        p = (uint32_t *)o->c; // (shortcut) start/current position of the blit
        int len = (*p & 0xffff)>>3;
        if (*p & 0x80000000)
            fast_fill(x,x+len,*p >> 16);
        x += len;
        o->c += 4;
    } while (!(*p & 1)); // stop if it was an eol
}


static void sprite_p4_line (object *o)
{
    int x=o->x;

    uint16_t *restrict pal = (uint16_t*) o->a; // shortcut
    uint32_t *p;
    do {
        uint32_t *restrict src = (uint32_t *)o->c;
        p=src;
        uint32_t w = *src++;


        x += SKIP(w); // skip : advance x

        // now, palette blit it
        uint32_t data_len = LEN(w); // in pixels
        uint32_t i=data_len; // remaining pixels to blit
        uint32_t *restrict dst = (uint32_t*) &draw_buffer[x&0xfffe]; // 2 pixels at a time

        if (x&1) { // non aligned = u16 aligned target ab bc cd de ...
            uint32_t a; // prec unblitted last pixel is in a

            // finish w
            a=*dst & 0x0000ffff; // little endian

            if (i>=4) { // at least 4 pixels : blit those 4 pixels
                *dst++ = a              | pal[w>>16&0xf]<<16;
                *dst++ = pal[w>>20&0xf] | pal[w>>24&0xf]<<16;
                a = pal[w>>28&0xf];
                i -=4; w=*src++; // load next word
            } else {
                w>>=16; // let the loop finish it, by realigning it
            }

            for (;i>=8;i-=8) // blit a source word
            {
                *dst++ = a              | pal[w>> 0&0xf]<<16;
                *dst++ = pal[w>> 4&0xf] | pal[w>> 8&0xf]<<16;
                *dst++ = pal[w>>12&0xf] | pal[w>>16&0xf]<<16;
                *dst++ = pal[w>>20&0xf] | pal[w>>24&0xf]<<16;
                a = pal[w>>28&0xf];
                w=*src++;
            }

            // finish with 1, 2..7 u16 pixels in the last word
            while (i>=2) { // first with couples (ie u32)
                *dst++ = a | pal[w&0xf] <<16;
                a = pal[w>> 4&0xf];
                i -=2; w>>=8;
            }

            if (i) { // a + 1 pixel
                *dst = a | pal[w&0xf]<<16;
            } else { // a + empty
                *dst = a | (*dst&0xffff0000) ;
            }
            dst++;


        } else { // aligned target
            if (i>=4) { // at least 4 pixels : blit those 4 pixels
                *dst++ = pal[w>>16&0xf] | pal[w>>20&0xf]<<16;
                *dst++ = pal[w>>24&0xf] | pal[w>>28&0xf]<<16;
                i -=4; w=*src++; // load next input word
            } else {
                w>>=16; // let the loop finish it, by realigning it
            }

            for (;i>=8;i-=8) // blit a source word
            {
                *dst++ = pal[w    &0xf] | pal[w>> 4&0xf]<<16;
                *dst++ = pal[w>> 8&0xf] | pal[w>>12&0xf]<<16;
                *dst++ = pal[w>>16&0xf] | pal[w>>20&0xf]<<16;
                *dst++ = pal[w>>24&0xf] | pal[w>>28&0xf]<<16;
                w=*src++;
            }

            // finish with 1, 2..7 u16 pixels int the last word
            while (i>=2) { // first with couples (ie u32)
                *dst++ = pal[w   &0xf] | pal[w>> 4&0xf]<<16;
                i -=2; w>>=8;
            }
            // last one is there is one - nec. left one
            if (i) {
                *dst = pal[w&0xf] | (*dst&0xffff0000) ;
                dst++;
            }
        }

        // advances x and pointer to next blit
        x += data_len; // next x
        o->c += 2+2*((data_len+3)/4);
    } while (!(*p & 1)); // stop if it was an eol

}

static void sprite_c8_line (object *o)
{
    int x=o->x & ~1; // force alignment on even pixel (u32 addresses)
    int i;
    uint32_t c;

    uint32_t *restrict pal = (uint32_t*) o->a; // shortcut

    uint32_t *restrict p; // sees blit as u32 made of 4 references. P is the start of the blit.
    do {
        p = (uint32_t *)o->c; // (shortcut) start/current position of the blit
        o->c += 4; // header word

        uint32_t data_len = LEN(*p); // in couples
        x += 2*SKIP(*p); // skip  is in couples

        uint32_t *restrict dst;
        uint32_t *restrict src;
        if (x>=0) {
            dst = (uint32_t*) &draw_buffer[x];
            src = p;

            // blit first two couples - if not skipped ! XXX not if x started at left and has been eaten ...
            if (data_len==0) continue;

            // handle transparency for first one
            c=pal[(*src>>16) & 0xff];
            *dst = c & 0x0000ffff ? c : c | (*dst & 0x0000ffff);
            dst++;
            if (data_len>=2) {
                // also test last pixel of second couple if it's exactly 3pixels wide
                c=pal[(*src>>24) & 0xff];
                *dst = c & 0xffff0000 ? c : c | (*dst & 0xffff0000);
                dst++;
                src++;
            }

            // now, palette blit middle couples, 4 couples (=1 input word) by 4
            for (i=data_len-2;i>4;i-=4, src++)
            {
                *dst++ = pal[*src>> 0 & 0xff];
                *dst++ = pal[*src>> 8 & 0xff];
                *dst++ = pal[*src>>16 & 0xff];
                *dst++ = pal[*src>>24 & 0xff];
            }

            // finish with 0-3 couples
            uint32_t s = *src; // last input couple
            switch (i)
            {
                // NB fall through
                case 4 :
                    *dst++ = pal[s & 0xff];
                    s >>=8;
                case 3 :
                    *dst++ = pal[s & 0xff];
                    s >>=8;
                case 2 :
                    *dst++ = pal[s & 0xff];
                    s >>=8;
                case 1 : // last one can be transparent - ie be 0 exactly
                    c = pal[s & 0xff];
                    *dst= c & 0xffff0000 ? c : c | (*dst & 0xffff0000);
            }
        } else { // <0 : dont display this run
            // always remove first two
            // FIXME: skip what is needed and blit what is needed to get back to right element
            // finish normally
            dst = (uint32_t*) &draw_buffer[0];


            /// XXX how many to blit to get sure that src is aligned to a word ?
        }
        // advance x
        x += data_len*2;

        // advances pointer to next blit
        o->c += 4*((data_len-2+3)/4);

        // skip a line. gets to the next blit & check eol
    } while (!(EOL(*p))); // stop if it was an eol

}


