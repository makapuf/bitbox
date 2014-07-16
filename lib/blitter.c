#include "blitter.h"

#include <stdint.h>
#include <stddef.h> // NULL
#include <string.h> // memset, size_t (indirect)

void *memcpy2 (void *dst, const void *src, size_t count); // included as memcpy in a later newlib version

typedef struct {
    // list of objects blit.

    object object_store[MAX_OBJECTS]; // real objects. set y to INT16_MAX for not used   
    // list of objects blit.
    object *objects[MAX_OBJECTS]; // Sorted by Y. Objects shall not be added or modified within display
    int nb_objects; // next unused object. There maybe be unused objects before.


    // active list is the list of object currently being blit. 
    // an object is not active if the displayed line is before its topmost line, or after the bounding box.
    object *activelist_head; // top of the active list
    int next_to_activate; // next object to put in active list

} Blitter;

Blitter blt __attribute__ ((section (".ccm")));

void blitter_init()
{   
    // initialize empty objects array
    memset(&blt,0,sizeof(blt));
    for (int i=0;i<MAX_OBJECTS;i++) 
    {
        blt.objects[i]=&blt.object_store[i]; 
        blt.objects[i]->y=INT16_MAX;
    }
    blt.nb_objects = 0; // first unused is first.
}

// return ptr to new object
// append to end of list ; list ends up unsorted now
object* blitter_new() 
{
    if (blt.nb_objects<MAX_OBJECTS)
        return blt.objects[blt.nb_objects++]; // index of free object IN !
    else 
        return 0;
} 

void blitter_remove(object *o)
{
    // ! Should do that between frames (only frame-based variables will be updated)
    o->y = INT16_MAX; 
}

// http://en.wikipedia.org/wiki/Insertion_sort
// insertion sort of object ptr array, sorted by Y. Simplest form, just sorting.
void blitter_sort_objects_y()
{
    // consider empty values as bigger than anything (Y = INT16_MAX)

    object *valueToInsert; 
    int holePos;
    int real_nbobjects = blt.objects[0]->y==INT16_MAX ? 0 : 1; // count the real number of objects. start at 0 or 1

    // The values in blt.objects[i] are checked in-order, starting at the second one
    for (int i=1;i<blt.nb_objects;i++) 
    {
        // at the start of the iteration, objects[0..i-1] are in sorted order
        // this iteration will insert blt.objects[i] into that sorted order
        // save blt.objects[i], the value that will be inserted into the array on this iteration
        valueToInsert = blt.objects[i];

        // now mark position i as the hole; blt.objects[i]=blt.objects[holePos] is now considered empty
        holePos=i;

        // keep moving the hole down until the valueToInsert is larger than 
        // what's just below the hole or the hole has reached the beginning of the array
        // null pointers are considered larger than anything (except null pointers but we've made the test already)
        
        //while (holePos > 0 && (!blt.objects[holePos - 1] || valueToInsert->y < blt.objects[holePos - 1]->y))        
        while (holePos > 0 && (valueToInsert->y < blt.objects[holePos - 1]->y))
        { //value to insert doesn't belong where the hole currently is, so shift 
            blt.objects[holePos] = blt.objects[holePos - 1]; //shift the larger value up
            holePos -= 1;       //move the hole position down
        }

        // hole is in the right position, so put valueToInsert into the hole
        blt.objects[holePos] = valueToInsert;
        
        if (valueToInsert->y != INT16_MAX) real_nbobjects+=1; // count number of real objects (skipped the holes)

        // blt.objects[0..i] are now in sorted order
    }
    blt.nb_objects=real_nbobjects;
}


void blitter_frame()
{
    // ensure objectlist is sorted by y 
    blitter_sort_objects_y();

    // reset activelist, next to blit
    blt.activelist_head = (object*)0;
    blt.next_to_activate = 0;

    // rewind all objects. nb objects is up to date (no holes in objlist, since we just sorted them)
    object *o;
    for (int i=0;i<blt.nb_objects;i++)
    {
        o=blt.objects[i];
        if (o->frame)
            o->frame(o,o->y<0?-o->y:0); // first line is -y if negative
    }
}


void activelist_add(object *o)
// insert sorted
{
    object **head = &blt.activelist_head;
    // find insertion point
    while (*head && (*head)->z > o->z) 
        head = &(*head)->activelist_next;

    // insert effectively
    o->activelist_next = *head;
    *head = o;
} 



void blitter_line()
{
    // XXX optimize : combine three loops in one ?

    // drop past objects from active list.
    object *prev=NULL;
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        if (vga_line >= o->y+o->h) 
        {           
            // remove this object from active list
            if (o==blt.activelist_head)
            {   // change head ?
                blt.activelist_head = o->activelist_next;
            } else {
                prev->activelist_next=o->activelist_next;
            }
        } else {
            prev=o;
        }
    }

    
    // add new active objects
    while (blt.next_to_activate<blt.nb_objects && (int)vga_line>=blt.objects[blt.next_to_activate]->y)
    {
        activelist_add(blt.objects[blt.next_to_activate]);
        //printf("activate %d\n",blt.objects[blt.next_to_activate]->x);
        blt.next_to_activate++;
    }


    // now trigger each activelist, in Z descending order
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        if (vga_line<o->y+o->h) // XXX FIXME ???
            o->line(o); // will call pre_draw and blit opaques
    }
}



void fast_fill(uint16_t x1, uint16_t x2, uint16_t c)
{   
    // ensures start is 32bit-aligned
    if (x1 & 1) 
    {
        draw_buffer[x1]=c; 
        x1++;
    }
    
    // ensures end is written if unaligned
    if (!(x2 & 1)) {
        draw_buffer[x2]=c; // why this +1 ????
        x2--;
    }
    
    // 32 bit blit, manually unrolled
    uint32_t * restrict dst32 = (uint32_t*)&draw_buffer[x1];
    int i=(x2-x1)/2;
    
    for (;i>=8;i-=8)
    {
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
        *dst32++ = c<<16 | c;
    }
    
    for (;i>=0;i--)
        *dst32++ = c<<16 | c; 
} __attribute__((hot))


// --- misc implementations & tests 

void dummy_frame(object *o,int first_line) {}

void color_blit(object *o) 
{
    fast_fill(
        o->x<0 ? 0 : o->x,
        o->x+o->w>640?640:o->x+o->w,
        o->a
        ); 
}


object *rect_new(int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color) 
// faire un sprite RLE ?
{
    object *o = blitter_new();
    if (!o) return 0; // error

    o->x=x; o->y=y; o->z=z;
    o->w=w; o->h=h;

    o->a = (int)color;
    
    o->frame=dummy_frame;
    o->line=color_blit;
    return o;
}

// degrade vertical (==rect ? a/b : plus joli, ~ gratuit)


// -------------------------------------------
/* mask data : to be used with a fill function. uses a & b variables of objects 
 *  mask is a uint16_t separate array, pointed by a. b is an internal state variable.
 *  bitmap data can be implmented differently : raw, DXT data, ...
 *  easier to combine if blit can be defined independently, ie can determine source data uniquely fom x
 * 
 *  list of skip/blit alternates, with eol bit. start with skip
 *  mask_record : u16 : skip:7, eol:1, data:8  
 *  fin du masque determine par y/nb lines
 */

/*
a record is made of
- u32 type ID
- u32 size in bytes of record, excluding. Size is always a 4 multiple
- void* data[size]

a sprite in memory is made of those records. 
  Header : type,w,h,frames
  Line16 : u16 index_of_words starting each lines within data
  u16 : u16 data[] (skip:8, blit:7,eol:1, u16[]) * (aligned to words)
  p4 : + u32 palette 4 bits indices [] (skip:8, blit:7,eol:1, w:4[])
  p8 : idem, 8 bits
  palette : u16[]

Object storage : 
    data : pixel data 
    a : palette (not used if u16)
    b : line16 skips 
    c : current blitting pos in data

*/

// this is internal
enum sprite_recordID {
    sprite_recordID__header=0,
    sprite_recordID__palette=1,
    sprite_recordID__line16=2,
    sprite_recordID__u16=1001,
    sprite_recordID__p4=1002,
    sprite_recordID__p8=1003,
    sprite_recordID__end=32767,
};

static void sprite_u16_frame(object *o, int start_line);
static void sprite_u16_line(object *o);

static void sprite_p8_frame(object *o, int start_line) {}
static void sprite_p8_line(object *o) {}

static void sprite_p4_frame(object *o, int start_line);
static void sprite_p4_line(object *o);



object * sprite_new(uint32_t *sprite_data) 
{
    object *o = blitter_new();
    if (!o) return 0; // error : no room left

    uint32_t t=-1, sz;

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

            case sprite_recordID__end : 
                break;

            case sprite_recordID__line16 : 
                o->b = (uintptr_t)sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__p8 : 
                o->frame = sprite_p8_frame;
                o->line = sprite_p8_line;
                o->data = sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read ; incl padded
                break;
            
            case sprite_recordID__p4 : 
                o->frame = sprite_p4_frame;
                o->line = sprite_p4_line;
                o->data = sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            case sprite_recordID__u16 : 
                o->frame = sprite_u16_frame;
                o->line = sprite_u16_line;
                o->data = sprite_data;
                sprite_data += (sz+3)/4; // skip, don't read
                break;

            default : 
                return 0; // error : unknown record !
                break;
        }
    }
    return o;
}



static void sprite_u16_frame (object *o, int start_line)
{
    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines

    start_line += o->fr*o->h; // FIXME : frame ? add line offset (used for multiframe sprites ...) to start line

    // ffwd current pointer to nearest preceding 16 line start 
    o->c = (intptr_t)o->data + ((uint16_t*)o->b)[start_line/16]*4;

    /*
    
    TODO

    start_line %= 16; // remainder
    while (start_line) {
        p = (uint16_t *)o->c; 
        if (pb_eol(p)) start_line--;
        // advances pointer to next blit anyway (as integer, so byte offsets)
        o->c += 2; // header
        o->c += 2*pb_copy(p);
        if (!pb_skip(p)) o->c += 2; 
    }  
    */
}


static void sprite_u16_line (object *o)
{
    int x=o->x;  
    uint16_t *p; // see blit as half words. p is always the start of the blit
    do {
        p = (uint16_t *)o->c; // (shortcut) start/current position of the blit

        x += (*p)>>8; // skip : advance x 
        if (x>1024) while (1) {}; // XXX protection, remove me
            
        // now, copy blit it 
        uint32_t data_len = (*p>>1)&0x7f;

        memcpy2(&draw_buffer[x], p+1, 2*data_len); // source = u16 after p

        // advance x 
        x += data_len;

        // advances pointer to next blit
        o->c += 2; // data len
        o->c += 2*data_len;

        // skip a line. gets to the next blit & check eol
    } while (!(*p & 1)); // stop if it was an eol

}


static void sprite_p4_frame (object *o, int start_line)
// XXX share with u16 ?
{
    // start line is how much we need to crop to handle out of screen data
    // also handles skipped lines

    start_line += o->fr*o->h; // FIXME : frame ? add line offset (used for multiframe sprites ...) to start line

    // ffwd current pointer to nearest preceding 16 line start 
    o->c = (intptr_t)o->data + ((uint16_t*)o->b)[start_line/16]*4;

    /*
    
    TODO

    start_line %= 16; // remainder
    while (start_line) {
        p = (uint16_t *)o->c; 
        if (pb_eol(p)) start_line--;
        // advances pointer to next blit anyway (as integer, so byte offsets)
        o->c += 2; // header
        o->c += 2*pb_copy(p);
        if (!pb_skip(p)) o->c += 2; 
    }  
    */
}


static void sprite_p4_line (object *o)
{
    int x=o->x;  // force event 
    int i;

    x&=~1;
    uint16_t *pal = (uint16_t*) o->a; // shortcut
    uint16_t *p; // sees blit as u16 made of 4 references. p is always the start of the blit
    do {
        p = (uint16_t *)o->c; // (shortcut) start/current position of the blit

        x += (*p)>>8; // skip : advance x 
        if (x>1024) while (1) {};
            
        // now, palette blit it 
        uint32_t data_len = (*p>>1)&0x7f; // in pixels
        uint16_t *src = p+1;
        uint32_t *dst = (uint32_t*) &draw_buffer[x]; // 2 pixels at a time

        for (i=data_len;i>4;i-=4)
        {
            *dst++ = pal[*src   &0xf] | pal[*src>> 4&0xf]<<16;
            *dst++ = pal[*src>>8&0xf] | pal[*src>>12&0xf]<<16;
            src++;
        }

        // finish with 1, 2, 3 or 4 u16 pixels ..
        while (i>2) {
            *dst++ = pal[*src   &0xf] | pal[*src>> 4&0xf]<<16;
            i -=2;
        }

        if (i) 
            *dst = pal[*src>>8&0xf] | (*dst&0xffff0000) ;
        

        src++;

        // advance x 
        x += data_len;

        // advances pointer to next blit
        o->c += 2; // data len
        o->c += 2*((data_len+3)/4);

        // skip a line. gets to the next blit & check eol
    } while (!(*p & 1)); // stop if it was an eol

}


// --- BTC4 (single and 2x magnification)
// ---------------------------------------------------------------------------

void btc4_line (object *o);

object * btc4_new(const uint32_t *btc, int16_t x, int16_t y, int16_t z)
{
    object *o = blitter_new();    
    if (!o) return 0; // error

    // generic attributes
    o->x=x; o->y=y;o->z=z;

    o->w=*btc++;
    o->h=*btc++;

    // palette start + blocks
    o->data = (uint32_t*)btc;
    
    o->line=btc4_line;
    return o;
}

// switch16 version (fastest so far. could be made faster by coding to ASM (?) or blitting 4 lines at a time - full blocks, loading palette progressively ...)
void btc4_line (object *o)  
{
    int line=vga_line-o->y;
    uint16_t *palette = (uint16_t*)(o->data);

    // palette is 256 u16 so 128 u32 after start (no padding)
    // data advances width/4 words per block (and a block is line/4)
    uint32_t *src =  ((uint32_t*)(o->data)) + 128 + (o->w/4)*(line / 4); 
    //uint32_t linemask = 3<<((line&3)*4); // test of bits starts with this mask, 16,20,24,28

    int x = (o->x) & 0xfffffffe; // ensure word aligned ... case unaligned TBD
    uint32_t *dst = (uint32_t*) (&draw_buffer[x]); 
    int n=o->w/4;

    // __builtin_expect(((n > 0) && ((n&7)==0));
    for (int i=0;i<n;i++)
    {
        uint32_t word_block = *src++; 

        uint16_t c1=palette[ word_block >>24]; 
        uint16_t c2=palette[(word_block >>16) & 0xff]; 

        switch(word_block>>((line&3)*4) & 0xf)
         {
            case   0 : *dst++ = (c2<<16)|c2; *dst++ = (c2<<16)|c2; break;
            case   1 : *dst++ = (c2<<16)|c1; *dst++ = (c2<<16)|c2; break;
            case   2 : *dst++ = (c1<<16)|c2; *dst++ = (c2<<16)|c2; break;
            case   3 : *dst++ = (c1<<16)|c1; *dst++ = (c2<<16)|c2; break;
            case   4 : *dst++ = (c2<<16)|c2; *dst++ = (c2<<16)|c1; break;
            case   5 : *dst++ = (c2<<16)|c1; *dst++ = (c2<<16)|c1; break;
            case   6 : *dst++ = (c1<<16)|c2; *dst++ = (c2<<16)|c1; break;
            case   7 : *dst++ = (c1<<16)|c1; *dst++ = (c2<<16)|c1; break;
            case   8 : *dst++ = (c2<<16)|c2; *dst++ = (c1<<16)|c2; break;
            case   9 : *dst++ = (c2<<16)|c1; *dst++ = (c1<<16)|c2; break;
            case  10 : *dst++ = (c1<<16)|c2; *dst++ = (c1<<16)|c2; break;
            case  11 : *dst++ = (c1<<16)|c1; *dst++ = (c1<<16)|c2; break;
            case  12 : *dst++ = (c2<<16)|c2; *dst++ = (c1<<16)|c1; break;
            case  13 : *dst++ = (c2<<16)|c1; *dst++ = (c1<<16)|c1; break;
            case  14 : *dst++ = (c1<<16)|c2; *dst++ = (c1<<16)|c1; break;
            case  15 : *dst++ = (c1<<16)|c1; *dst++ = (c1<<16)|c1; break;
        }
    }
} //__attribute__ ((hot))

// btc4_2x for pixel_doubled data

void btc4_2x_line (object *o);

object * btc4_2x_new(const uint32_t *btc, int16_t x, int16_t y, int16_t z)
{
    object *o = blitter_new();    
    if (!o) return 0; // error

    // generic attributes
    o->x=x; o->y=y;o->z=z;

    o->w=(*btc++)*2;
    o->h=(*btc++)*2;

    // palette start + blocks
    o->data = (uint32_t*)btc;
    
    o->line=btc4_2x_line;
    return o;
}

// switch16 version (fastest so far. could be made faster by coding to ASM (?) or blitting 4 lines at a time - full blocks, loading palette progressively ...)
void btc4_2x_line (object *o)  
{
    int line=(vga_line-o->y)/2; // line into the buffer, zoomed 2x vertically
    uint16_t *palette = (uint16_t*)(o->data);

    // palette is 256 u16 so 128 u32 after start (no padding)
    // data advances width/8 *words* per blockline (and a block is line/4)
    uint32_t *src =  ((uint32_t*)(o->data)) + 128 + (o->w/8)*(line / 4); 

    int x = (o->x) & 0xfffffffe; // ensure word aligned ... case unaligned TBD
    uint32_t *dst = (uint32_t*) (&draw_buffer[x]); 
    int n=o->w/8;

    for (int i=0;i<n;i++) // FIXME should end n accoprding to start x ! 
    {
        uint32_t word_block = *src++; 

        uint32_t c1=palette[ word_block >>24]*0x00010001; // mul is done to repeat twice in the word
        uint32_t c2=palette[(word_block >>16) & 0xff]*0x00010001; 

        switch(word_block>>((line&3)*4) & 0xf) // read two pixels at a time
         {
            case   0 :  *dst++=c2;*dst++ = c2;  *dst++=c2;*dst++=c2; break;
            case   1 :  *dst++=c1;*dst++ = c2;  *dst++=c2;*dst++=c2; break;
            case   2 :  *dst++=c2;*dst++ = c1;  *dst++=c2;*dst++=c2; break;
            case   3 :  *dst++=c1;*dst++ = c1;  *dst++=c2;*dst++=c2; break;
            case   4 :  *dst++=c2;*dst++ = c2;  *dst++=c1;*dst++=c2; break;
            case   5 :  *dst++=c1;*dst++ = c2;  *dst++=c1;*dst++=c2; break;
            case   6 :  *dst++=c2;*dst++ = c1;  *dst++=c1;*dst++=c2; break;
            case   7 :  *dst++=c1;*dst++ = c1;  *dst++=c1;*dst++=c2; break;
            case   8 :  *dst++=c2;*dst++ = c2;  *dst++=c2;*dst++=c1; break;
            case   9 :  *dst++=c1;*dst++ = c2;  *dst++=c2;*dst++=c1; break;
            case  10 :  *dst++=c2;*dst++ = c1;  *dst++=c2;*dst++=c1; break;
            case  11 :  *dst++=c1;*dst++ = c1;  *dst++=c2;*dst++=c1; break;
            case  12 :  *dst++=c2;*dst++ = c2;  *dst++=c1;*dst++=c1; break;
            case  13 :  *dst++=c1;*dst++ = c2;  *dst++=c1;*dst++=c1; break;
            case  14 :  *dst++=c2;*dst++ = c1;  *dst++=c1;*dst++=c1; break;
            case  15 :  *dst++=c1;*dst++ = c1;  *dst++=c1;*dst++=c1; break;
        }
    }
} 

// --- 16x16 Tilemap
// --------------------------------------------------------------------------------------

// tileset is a list of 16x16 data  
// width and height are displayed data, can be bigger than tileset, in which case loop
// tilemap can be 32x32, 64x32. references can be u16, i16 (semi transparent tiles), u8 or i8

/*
    data : 
        first u32 : 
            tilemap_size:3 = 0: 64x64, ...
            tilemap_index_type:2 = 0:u16, 1:u8, 2:i16, 3:i8
            rest : RFU

        rest : tile_index either u8 or u16 ... 

    RAM data : 
    
        *data : start of tilemap (id data+1)
        a : tileset
        b : current tile in tilemap

 */



#define TILESIZE16 16
#define HEIGHT_64 64
#define WIDTH_64 64
#define vga_screen_width 640

#define min(a,b) (a<b?a:b)

void tilemap_6464u8_line(object *o)
{
    // in this version, we can assume that we don't have a full 
    // TODO : take care of smaller x, don't recalc all each time. case o->x <0 

    // use current frame, line, buffer

    // line inside tile (pixel), looped.
    int sprline = (vga_line-o->y) % (HEIGHT_64*16); 

    // index of first tile to run in tile map, in pixels
    int start_tile = o->x%(WIDTH_64*16)/16; 

    // which tile to start on 
    uint8_t *idxptr = (uint8_t *)o->data+(sprline/TILESIZE16) * WIDTH_64 + start_tile; // all is in nb of tiles

    // offset from start of tile (in lines)
    int offset = sprline%TILESIZE16; 

    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict dst = (uint32_t*) &draw_buffer[o->x+start_tile*TILESIZE16]; 
    int right_stop = min(o->x+o->w, vga_screen_width); // end at which pixel ? 

    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?
    
    uint32_t *tiledata = (uint32_t *)o->a;

    while (dst<dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        if (*idxptr) {
            uint32_t * restrict src;
            src = &tiledata[((*idxptr)*TILESIZE16 + offset)*TILESIZE16*2/4];  

            // force unroll
            *dst++=*src++;  
            *dst++=*src++;  
            *dst++=*src++;  
            *dst++=*src++;  

            *dst++=*src++;  
            *dst++=*src++;  
            *dst++=*src++;  
            *dst++=*src++;  

            /* __asm__ (
                "ldmia %[src]!,{r0-r7} \r\n"
                "stmia %[dst]!,{r0-r7}"
                :[src] "+r" (src), [dst] "+r" (dst)
                :: "r0","r1","r2","r3","r4","r5","r6","r7"
                );
            */
        } else { // skip the tile
            dst += TILESIZE16/2; // words per tile
        }
        idxptr++;
    }
}


object * tilemap_new(const uint16_t *tileset, int w, int h, uint32_t *tmap)
{
    object *o = blitter_new();    
    if (!o) return 0; // error


    // generic attributes
    o->x=0;
    o->y=500; // hidden by default
    o->z=0;

    o->w=w;  o->h=h;

    o->data = (uint32_t*)tmap+1; 
    o->a = (uintptr_t)tileset-512; // to start at index 1 and not 0, offset now in bytes.
    
    switch (*tmap)
    {
        case 0 : // 64x64, u8 references
            o->line=tilemap_6464u8_line;
            break;

        default : 
            message("Unknown format code !");
            return 0; // error : not implemented
            break;
    }
    return o;
}
