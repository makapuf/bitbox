#include <math.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "kernel.h"

#define SATURATE(x,a,b) ((x)>(a)?((x)<(b)?(x):(b)):(a))


// top level of sorted display list. Display list is a sorted list of non null, sorted by Z axis
Object *displaylist_top; // objects with compatible layout to object needed ! empty objects have null type.
extern const uint32_t tiledata[];

void play_sample(Sample *s)
{
}

// Objects -----------------------------------------------------------------------------------------------

inline void object_disable(Object *o)
{
    // dont disable subs. should be already killed.
    // o->x = 600;    o->y = 400;
    //printf("disabling %p (%s)\n",o,o->type->name);
    o->type=NULL;
}

inline void object_disable_children(Object *o)
{
    //printf("disabling children of %s\n",o->type->name);
    for (Object *op=displaylist_top;op;op=op->displaylist_next)
    {
        if (op->parent == o)
        {
            // un-parent child&disable
            //printf("disabling %s sub of %s\n",op->type->name,o->type->name);
            op->parent=0;
            op->type=0;
        }
    }
}

// TEMPLATE / specialize8 ?
int object_collide(const Object *o, int x, int y, int w, int h )
{
    // iterate columns of object within x1 and x2
    // iterate lines of object within y1 and y2
    // first non-empty tile : return true
    int x1=(x-o->x)/o->type->tilesize;
    int y1=(y-o->y)/o->type->tilesize;
    int x2=(x+w-o->x)/o->type->tilesize;
    int y2=(y+h-o->y)/o->type->tilesize;

    x1=SATURATE(x1,0,o->type->tilemap_width);
    x2=SATURATE(x2,0,o->type->tilemap_width);
    y1=SATURATE(y1,0,o->type->tilemap_height);
    y2=SATURATE(y2,0,o->type->tilemap_height);

    uint16_t* start_frame = (uint16_t*)o->type->frames + o->anim_frame*o->type->tilemap_height*o->type->tilemap_width;

    for (int lig=y1;lig<y2;lig++)
        for(int col=x1;col<x2;col++)
        {
            if (start_frame[lig*o->type->tilemap_width+col])
                return 1;
        }
    return 0;
}



// blit avec transp, translucide,  transp, colorise
// ASSUMES LITTLE ENDIAN
#define TILESIZE8 8
void object_blit8(Object *o)
{
    // use current frame, line, buffer

    const ObjectType *ot = o->type;
    int x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE8*ot->tilemap_height)) return;

    //if (x<-MARGIN || x>640+MARGIN) return;
    // printf ("line%d, frame %d, tile: %d\n",line, frame, frame*ot->tilemap_height*ot->tilemap_width + sprline/ot->tilesize);


    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE8; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE8)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE8)*TILESIZE8/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t *src;
    uint32_t *dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*8];

    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd

    const unsigned int ntiles = ot->tilemap_width; // clipping right done in loop
    uint32_t a;

    for (int i = start_tile;i<ntiles && (void*)dst< (void*)&draw_buffer[screen_width+MARGIN] ;i++) // XXX cut is a const in frame !
    {
        // blit one tile, 32 bits at a time, 4times = 8pixels
        if (*idxptr) {
            src = (uint32_t*)&tiledata[(*idxptr)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const
            // alignment divided by two since we use 32bits words ie 2 pixels.

            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
        } else { // no tile, skip the pixels
            dst += TILESIZE8>>1;
        }
        idxptr++;
    }
    // printf("\n");
}


void object_blit8_opaque(Object *o)
{
    // use current frame, line, buffer

    const ObjectType *ot = o->type;
    int x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE8*ot->tilemap_height)) return;

    //if (x<-MARGIN || x>640+MARGIN) return;
    // printf ("line%d, frame %d, tile: %d\n",line, frame, frame*ot->tilemap_height*ot->tilemap_width + sprline/TILESIZE16);


    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE8; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE8)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE8)*TILESIZE8/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t *src;
    uint32_t *dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*8];

    //dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd

    const unsigned int ntiles = ot->tilemap_width; // clipping right done in loop

    for (int i = start_tile;i<ntiles && (void*)dst< (void*)&draw_buffer[screen_width+MARGIN] ;i++) // XXX cut is a const in frame !
    {
        // blit one tile, 32 bits at a time, 4times = 8pixels
        if (*idxptr) {
            src = (uint32_t*)&tiledata[(*idxptr)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const
            // alignment divided by two since we use 32bits words ie 2 pixels.
            for (int j=4;j!=0;j--)
                *dst++=*src++;
        } else { // no tile, skip the pixels
            dst += TILESIZE8>>1;
        }
        idxptr++;
    }
    // printf("\n");
}


void old_object_blit16(Object *o)
{
    // use current frame, line, buffer

    const ObjectType *ot = o->type;
    int16_t x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE16*ot->tilemap_height)) return;

    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE16; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE16)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE16)*TILESIZE16/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict src;
    uint32_t * restrict dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*TILESIZE16];

    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?

    const unsigned int ntiles = ot->tilemap_width; // NO : CLIPPING RIGHT
    uint32_t a;

    for (int i = start_tile;i<ntiles && (void*)dst<(void*)&draw_buffer[screen_width+MARGIN];i++) // XXX cut -- const for a frame !
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        //printf(" i %d ref %x - ",i,*idxptr, src,*src,dst);
        if (*idxptr) {
            src = (uint32_t*)&tiledata[(*idxptr)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const

            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels

            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels
            a=*src++; if (a) {  *dst++ = a; } else { dst++; } // tests 2 pixels

        } else { // skip the pixels
            dst += TILESIZE16>>1;
        }
        idxptr++;
    }
    // printf("\n");
}


void object_blit16_bg(Object *o) // fixed size, no transp tile
{
    // use current frame, line, buffer

    const ObjectType *ot = o->type;
    int16_t x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE16*ot->tilemap_height)) return;

    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE16; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE16)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE16)*TILESIZE16/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict src;
    uint32_t * restrict dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*TILESIZE16];
    int right_stop = min(x+MARGIN+ot->tilemap_width*TILESIZE16, screen_width+MARGIN); // end at which pixel ?
    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?

    while (dst<=dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        src = (uint32_t*)&tiledata[(*idxptr++)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const
        // alignment divided by two since we use 32bits words ie 2 pixels.
        __asm__ (
                "ldmia %[src]!,{r0-r7} \r\n"
                "stmia %[dst]!,{r0-r7}"
                :[src] "+r" (src), [dst] "+r" (dst)
                :: "r0","r1","r2","r3","r4","r5","r6","r7"
                );
        
        //for (int j=0;j<TILESIZE16/2;j++) *dst++=*src++; // unrolled
        
    }
}



 void object_blit16_opaque(Object *o)
{
    // use current frame, line, buffer

    const ObjectType *ot = o->type;
    int16_t x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE16*ot->tilemap_height)) return;

    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE16; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE16)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE16)*TILESIZE16/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict src;
    uint32_t * restrict dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*TILESIZE16];
    int right_stop = min(x+MARGIN+ot->tilemap_width*TILESIZE16, screen_width+MARGIN); // end at which pixel ?
    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?

    while (dst<dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        if (*idxptr) {
            src = (uint32_t*)&tiledata[(*idxptr)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const
            // alignment divided by two since we use 32bits words ie 2 pixels.
            // for (int j=0;j<TILESIZE16/2;j++) *dst++=*src++; // unrolled
            __asm__ (
                "ldmia %[src]!,{r0-r7} \r\n"
                "stmia %[dst]!,{r0-r7}"
                :[src] "+r" (src), [dst] "+r" (dst)
                :: "r0","r1","r2","r3","r4","r5","r6","r7"
                );


        } else { // skip the tile
            dst += TILESIZE16*2/4;
        }
        idxptr++;
    }
}


void object_blit16(Object *o)
{
    // use current frame, line, buffer
    uint32_t a;
    const ObjectType *ot = o->type;
    int16_t x=o->x, y=o->y; // dx type / path ?

    int sprline = line-y; // line inside (pixel)
    if ((sprline<0) || (sprline>=TILESIZE16*ot->tilemap_height)) return;

    // index of first tile to run in tile map
    uint16_t start_tile = max((0-x),0)/TILESIZE16; // which tile to start on - from beginning of sprite 0 if column 0, try others to test
    uint16_t *idxptr = (uint16_t*)ot->frames + (o->anim_frame*ot->tilemap_height+ sprline/TILESIZE16)*ot->tilemap_width + start_tile;

    // offset from start of tile (in words)
    int offset = (sprline%TILESIZE16)*TILESIZE16/2; // line prec+tilesize8/2
    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict src;
    uint32_t * restrict dst = (uint32_t*) &draw_buffer[x+MARGIN+start_tile*TILESIZE16];
    int right_stop = min(x+MARGIN+ot->tilemap_width*TILESIZE16, screen_width+MARGIN); // end at which pixel ?
    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?

    while (dst<dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        if (*idxptr) {
            src = (uint32_t*)&tiledata[(*idxptr)*(TILEDATA_ALIGN /4)+offset];  // cast to remove const
            // alignment divided by two since we use 32bits words ie 2 pixels.
            for (int j=0;j<TILESIZE16/2;j++) {
                // unrolled
                a=*src++;
                if (a) 
                    {  *dst++ = a; } 
                else 
                    { dst++; } // tests 2 pixels
            }
        } else { // skip the tile
            dst += TILESIZE16>>1;
        }
        idxptr++;
    }
}

/** Display list is always z-sorted. sometimes objects in it are dead, ie have no type
*/

void displaylist_insert(Object *o)
/**
 insert an already active object to the display list, modifying top/bottom
*/
{
    // uninitialized : the only new element is the top/bottom
    if (!displaylist_top) {
        displaylist_top = o;
    }
    // new top ?
    else if (o->type->zlevel > displaylist_top->type->zlevel)
    {
        o->displaylist_next=displaylist_top;
        displaylist_top=o;
    }
    else
    {
        // go through displaylist, continue while end of list not reached or current element higher than object z
        Object *current=displaylist_top;
        while (current->displaylist_next && current->displaylist_next->type->zlevel > o->type->zlevel ) {
            current = current->displaylist_next;
        }
        // now the next is higher than object z
        // insert after current


        o->displaylist_next = current->displaylist_next;
        current->displaylist_next = o;

    }
}

void displaylist_remove(Object *o)
{
    Object *p;
    // remove top -> new top is next
    if (o==displaylist_top)
        displaylist_top=o->displaylist_next;
    else
    {
        // find parent : start from top (can be long : better do it once per frame max.)
        for (p=displaylist_top;p->displaylist_next!=o;p=p->displaylist_next);
        p->displaylist_next=o->displaylist_next;
    }
}

void displaylist_clean(Object *o)
/**
 Sweep remove all dead objects from displaylist.
 Enables to do it once per frame.
*/
{
    Object *p;

    // ensure top is alive or empty
    while (displaylist_top && !displaylist_top->type)
    {
        displaylist_top=displaylist_top->displaylist_next;
    }
    if (!displaylist_top) return; // empty list has no deads !

    p=displaylist_top;     // now list top is alive
    while (p->displaylist_next) // while there is a next one
    {
        // skip all next empty
        while (p->displaylist_next && !p->displaylist_next->type)
        {
            // replace dead next one with next of next (ie while next is dead, skip it)
            p->displaylist_next=p->displaylist_next->displaylist_next;
        }
        if (p->displaylist_next) p=p->displaylist_next; // next one if there is one
    }
}

void inline displaylist_draw()
// draw a line of all objects. displaylist must be sorted and purged
// could use some optimization, by skipping obviously out list, keeping a y sorted list, ...
// prepare objects before ?
{
    for (Object *o=displaylist_top;o;o=o->displaylist_next)
    {
        if (o->type->draw)
            o->type->draw(o); 
    }
}


Object * displaylist_collide(const Object *o,int z_start,int z_end) // zmin,zmax, included
{
    Object *p;

    /* skip displaylist with zlevel below object */
    for (
         p=displaylist_top;
         p->type && p->type->zlevel>z_end;
         p=p->displaylist_next
         );

    /* test each object not above zlevel */
    for (;p->type && p->type->zlevel >= z_start;p=p->displaylist_next)
    {
        // FIXME test collision, if collide, return it
        if (p!=o &&
            p->type->nframes &&
            object_collide(
               p, o->x, o->y,
               // box !
               o->type->tilemap_height*o->type->tilesize,
               o->type->tilemap_width*o->type->tilesize
               )
            )
            return p;
    }
    return 0; // nothing found
}