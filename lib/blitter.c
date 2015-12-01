#include "blitter.h"

#include <stdint.h>
#include <stddef.h> // NULL
#include <string.h> // memset

#ifndef EMULATOR
#include "stm32f4xx.h" // profile
#endif 

extern int line_time;

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

static int blitter_initialized = 0;

void blitter_init()
{   
    // initialize empty objects array
    memset(&blt,0,sizeof(blt));
    for (int i=0;i<MAX_OBJECTS;i++) 
    {
        blt.objects[i]=&blt.object_store[i]; 
        blt.objects[i]->ry=INT16_MAX;
    }
    blt.nb_objects = 0; // first unused is first.
    blitter_initialized=1;
}

// return ptr to new object
// append to end of list ; list ends up unsorted now
object* blitter_new() 
{
    // auto initialize in case it wasn't done
    if (!blitter_initialized) 
        blitter_init();

    if (blt.nb_objects<MAX_OBJECTS) {
        return blt.objects[blt.nb_objects++]; // index of free object IN !
    } else {
        message ("Object memory full, too many objects ! Increase MAX_OBJECTS in lib/blitter.h");
        return 0;
    }
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
    int real_nbobjects = blt.objects[0]->ry==INT16_MAX ? 0 : 1; // count the real number of objects. start at 0 or 1

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
        while (holePos > 0 && (valueToInsert->ry < blt.objects[holePos - 1]->ry))
        { //value to insert doesn't belong where the hole currently is, so shift 
            blt.objects[holePos] = blt.objects[holePos - 1]; //shift the larger value up
            holePos -= 1;       //move the hole position down
        }

        // hole is in the right position, so put valueToInsert into the hole
        blt.objects[holePos] = valueToInsert;
        
        if (valueToInsert->ry != INT16_MAX) real_nbobjects+=1; // count number of real objects (skipped the holes)

        // blt.objects[0..i] are now in sorted order
    }
    blt.nb_objects=real_nbobjects;
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



void graph_frame()
{
    if (!blitter_initialized) 
        return; // ensure initiliaztion is done

    object *o;

    // transfer y to real y ry
    for (int i=0;i<blt.nb_objects;i++)
    {
        o=blt.objects[i];  
        if (o->y==INT16_MAX || o->y+(int)o->h>=0) 
            o->ry = o->y;
        else
            o->ry = VGA_V_PIXELS+1; 
            // if hidden above screen, hide below screen (thus activation algorithms will never run)
    }

    // ensure objectlist is sorted by y 
    blitter_sort_objects_y();

    // reset activelist, next to blit
    blt.activelist_head = (object*)0;
    blt.next_to_activate = 0;

    // rewind all objects. nb objects is up to date (no holes in objlist, since we just sorted them)
    for (int i=0;i<blt.nb_objects;i++)
    {
        o=blt.objects[i];
        if (o->frame)
            o->frame(o,o->ry<0?-o->ry:0); // first line is -y if negative
    }
}





void graph_line()
{
    if (!blitter_initialized) 
        return; // ensure initiliaztion is done

    // persist between calls so that one line can continue blitting next frame.
    static object *o; 

    #ifdef VGA_SKIPLINE
    if (!vga_odd) { // only on even lines
    #endif 

    // drop past objects from active list.
    object *prev=NULL;
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        if (vga_line >= o->ry+o->h) 
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
    while (blt.next_to_activate<blt.nb_objects && (int)vga_line>=blt.objects[blt.next_to_activate]->ry)
    {
        // only if not hidden (Y too negative)
        activelist_add(blt.objects[blt.next_to_activate]);
        //printf("activate %d\n",blt.objects[blt.next_to_activate]->x);
        blt.next_to_activate++;
    }

    // now trigger each activelist, in Z descending order
    for (o=blt.activelist_head;o;o=o->activelist_next)
    {
        if (vga_line<o->ry+o->h) // XXX when/why does that not arrive ?
            o->line(o); 

        #ifndef EMULATOR
        // stop if cycle counter is big enough, dropping objects on that line - may continue after
        //if (DWT->CYCCNT - line_time > VGA_H_PIXELS*VGA_PIXELCLOCK-1000) break; 
        #endif 
    } 

    #ifdef VGA_SKIPLINE
    } else {
        // continue with o 
        for (;o;o=o->activelist_next)
        {
            if (vga_line<o->ry+o->h) // XXX when/why does that arrive ?
                o->line(o); 
        } 
    }
    #endif

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

    o->x=x; o->ry=y; o->z=z;
    o->w=w; o->h=h;

    o->a = (int)color;
    
    o->frame=dummy_frame;
    o->line=color_blit;
    return o;
}

// degrade vertical (==rect ? a/b : plus joli, ~ gratuit)

