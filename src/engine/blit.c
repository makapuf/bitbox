// see description of this algorithm in http://bitboxconsole.blogspot.fr/2013/10/an-efficient-scanline-blitting-algorithm.html

/* TODO : 
include DXT
include tilesprite
make a font element (avec sub packbits : array de chars = packbits independants, taille H donnee)
*/
#include "blit.h"

#include <stdint.h>
#include <stddef.h> // NULL

// XXX Add some locking within frame ? 


/*

 lists : object array characteristics
 object list : sorted y, slowly inserted / removed, can be big <- as array. 
    optimisation+ tard : keep holes / linked list
 
 active list : allocate and insert semifast. need to remove also. insert : 1/line

 blit : FAST, ~ size active, kept sorted, emptied auto, no remove, insert,. frequent (N times/line) allocate & insert sorted. no re-sorting
    ~16 max ? 

 lifo transp stack : fast, limited?), emptied never popped, pushed seq : an array .

 */

typedef struct {
    // ** long standing variables

    // list of objects blit.
    int nb_objects; // maximum used objects. There may be holes before this however.
    object *objects[MAX_OBJECTS]; // Sorted by Y. Objects shall not be added or modified within display

    // ** frame-lived variables (ie they are reset each frame)

    int current_line; // current line blitting

    // active list is the list of object currently being blit. 
    // an object is not active if the displayed line is before its topmost line, or after the bounding box.
    object *activelist_head; // top of the active list
    int next_to_activate; // next object to put in active list

} Blitter;

// global local variable
static Blitter blt;

void blitter_init()
{   
    // initialize empty objects array
    for (int i=0;i<MAX_OBJECTS;i++) 
    {
        blt.objects[i]=&real_objects[i]; 
        real_objects[i].y=INT16_MAX;
    }
    blt.nb_objects = 0; // first unused is first.
}

// return new object pointer
// append to end of list ; list ends up unsorted now
// DON'T insert something twice !
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
// insertion sort objects array by Y. Simplest form, just sorting
void blitter_sort_objects_y()
{
    // consider empty values as bigger than anything (Y = INT16_MAX)

    object *valueToInsert; 
    int holePos;
    int real_nbobjects = 0; // count the real number of objects

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
        while (holePos > 0 && (!blt.objects[holePos - 1] || valueToInsert->y < blt.objects[holePos - 1]->y))
        { //value to insert doesn't belong where the hole currently is, so shift 
            blt.objects[holePos] = blt.objects[holePos - 1]; //shift the larger value up
            holePos -= 1;       //move the hole position down
        }

        // hole is in the right position, so put valueToInsert into the hole
        blt.objects[holePos] = valueToInsert;
        
        real_nbobjects+=1; // count number of real objects (skipped the holes)

        // blt.objects[0..i] are now in sorted order
    }
    blt.nb_objects=real_nbobjects;
}



void blitter_frame()
{
    // ensure objectlist is sorted by y 
    blitter_sort_objects_y();

    blt.current_line = 0;
    // reset activelist, next to blit
    blt.activelist_head = (object*)0;
    blt.next_to_activate = 0;

    // rewind all objects. nb objects is up to date (no holes, since we just sorted them)
    object *o;
    for (int i=0;i<blt.nb_objects;i++)
    {
        o=blt.objects[i];
        o->frame(o,o->y<0?-o->y:0); // first line is -y if negative
    }
}


void activelist_add(object *o)
// insert sorted by Z
{
    object **head = &blt.activelist_head;
    // find insertion point
    while (*head && (*head)->z < o->z) 
        head = &(*head)->activelist_next;

    // insert effectively
    o->activelist_next = *head;
    *head = o;
} 


void blitter_line()
{
    // drop past objects from active list. not sorted by Y
    object *prev=NULL;
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        if (blt.current_line >= o->y+o->h)
        {           
            // remove this object from active list
            if (o==blt.activelist_head)
            {   // change head ?
                blt.activelist_head = o->activelist_next;
            } else {
                prev->activelist_next=o->activelist_next;
            }
        }
        prev=o;
    }

    
    // add new active objects
    while (blt.next_to_activate<blt.nb_objects && blt.current_line>=blt.objects[blt.next_to_activate]->y)
    {
        activelist_add(blt.objects[blt.next_to_activate]);
        //printf("activate %d\n",blt.objects[blt.next_to_activate]->x);
        blt.next_to_activate++;
    }


    // now trigger each activelist, in Z descending order
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        o->line(o); 
    }

    // next line
    blt.current_line++;
}


// --- misc implementations & tests 

void dummy_frame(object *o,int first_line) {}

void opaquerect_line (object *o)
{
    pre_draw(o, o->x,o->x+o->w,OPAQUE);
}

void color_blit(object *o, int16_t x1, int16_t x2) 
{
    // memset of draw_buffer to color o->b
    //printf("BLIT(%d - %d)=%x\n",x1,x2,o->a);
    for (int i=x1;i<=x2;i++)
        draw_buffer[i]=o->a; // 32->16
}

// fixme colors from o
void checkers_blit(object *o, int16_t x1, int16_t x2) // unoptimized !!
{
    // unroll loop
    for (int i=x1;i<=x2;i++)
        draw_buffer[i] = ((blt.current_line/8)+(i/8))%2?(o->a&0xffff):(o->a>>16);  
}

int new_opaquerect(object *o, int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color)
{
    int i = blitter_insert((object *)o);    
    if (i<0) return i;

    // generic attributes
    o->x=x; o->y=y;
    o->h=h; o->w=w;
    o->z=z;

    // special attributes
    o->a = (int)color;
    
    o->frame=dummy_frame;
    o->line=opaquerect_line;
    o->blit=color_blit;
    return i;
}

int new_checkerrect(object *o, int16_t x, int16_t y, int16_t w, int16_t h,int16_t z, uint16_t color1, uint16_t color2)
{
    int i = blitter_insert((object *)o);    
    if (i<0) return i;

    // generic attributes
    o->x=x; o->y=y;
    o->h=h; o->w=w;
    o->z=z;

    // special attributes
    o->a = color1<<16 | color2;
    
    o->frame=dummy_frame;
    o->line=opaquerect_line;
    o->blit=checkers_blit;
    return i;
}


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
#define MASK_EOL (1<<7)
#define MASK_LEN(o)  (o->b & 0x7f)
#define MASK_SKIP(o) (o->b >> 9)

#define MASK_PTR(o) (o->b) // current pointer to mask data

inline void mask_frame(object *o, int start_line)
{
    uint16_t *p=(uint16_t *)o->a; // reset pointer to frame start.
    for (;start_line;start_line--) // scan to line. shall not be finished before. skips N eols
    {
        for (
            ;
            !(*p & MASK_EOL);
            p++
            ); // skip a line. gets to the next blit & check eol
    }
    o->b = (uintptr_t)p;
}

inline void mask_line(object *o)
{
    int x=o->x; // copy to the next 
    do {
        x += MASK_SKIP(o);
        pre_draw (o, o->x,o->x+MASK_LEN(o),OPAQUE); // will blit from ptr, maybe cut & restricted, maybe several times
        x += MASK_LEN(o);
        MASK_PTR(o) +=  sizeof(uint16_t); 
        // skip a line. gets to the next blit & check eol
    } while (*(uint16_t*)MASK_PTR(o) & MASK_EOL); // eol
}


// -- masked rle
// combines a packbit with a mask data
// skip lines ok, blit lines ok, combined with data
// advance data along 



// ---------------------------------------------------------------------------
