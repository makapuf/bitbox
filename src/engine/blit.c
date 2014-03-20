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



typedef struct transpblit{
    int16_t x1, x2;
    object *o;
} transpblit;

typedef struct {
    int16_t x1, x2;
} blit;


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
        

    // ** line-lived variables
    // list of x1,x2 opaque blits on the line. The list is non overlapping and has growing x. 
    blit opaque_list[MAX_BLITS]; 
    int opaque_list_nb; // number of active elements in the list. 

    transpblit transparent_stack[MAX_TRANSP]; // (x1,x2,b) fifo, filled top to bottom
    int transparent_stack_top; // stack top next free index, 0 == empty fifo
} Blitter;

// global localvariable
static Blitter blt;

void blitter_init()
{   
    for (int i=0;i<MAX_OBJECTS;i++) blt.objects[i]=0; // empty objects array
    blt.nb_objects = 0;
}

int blitter_insert(object *o)
// return blitter index for new object, negative if none found
// append to end of list ; list ends up unsorted now
// DON'T insert something twice !
{
    if (blt.nb_objects<MAX_OBJECTS)
    {
        blt.objects[blt.nb_objects++] = o;
        return blt.nb_objects;
    }
    else 
        return -1;
}

void blitter_remove(int i)
{
    // ! Should do that between frames (only frame-based variables will be updated)
    blt.objects[i]=0;   
}

// http://en.wikipedia.org/wiki/Insertion_sort
// insertion sort objects array by Y. Simplest form, just sorting
void blitter_sort_objects_y()
{
    // consider empty values as bigger than anything

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

        if (!valueToInsert) continue; // skip empty places, no need to sort them.

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

void transparent_push(int16_t x1, int16_t x2, object *o)
{
    if (blt.transparent_stack_top<MAX_BLITS) 
    {
        blt.transparent_stack_top+=1;
        blt.transparent_stack[blt.transparent_stack_top] = (transpblit){.x1=x1, .x2=x2, .o=o};
    } 
}


void activelist_add(object *o)
// insert sorted
{
    object **head = &blt.activelist_head;
    // find insertion point
    while (*head && (*head)->z < o->z) 
        head = &(*head)->activelist_next;

    // insert effectively
    o->activelist_next = *head;
    *head = o;
} 



/*
void print_objects()
{
    printf(" -- object list\n");
    object *o;
    for (int i=0;i<blt.nb_objects;i++)
    {
        o=blt.objects[i];
        if (o) {
            printf ("%d - x=%d y=%d w=%d h=%d z=%d a=%d\n",i ,o->x,o->y, o->w,o->h,o->z,o->a);
        } else {
            printf ("<empty>\n");
        }
    }
    printf(" --\n");
}

void print_opaquelist(void) 
{
    printf("[");
    for (int i=0;i<blt.opaque_list_nb;i++)
    {
        printf ("%3d.%3d,",blt.opaque_list[i].x1, blt.opaque_list[i].x2);
    }
    printf("]\n");
}

void print_active()
{
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
        printf (" x=%d y=%d w=%d h=%d z=%d a=%d\n",o->x,o->y, o->w,o->h,o->z,o->a); 
}


void print_frame(object *o,int line) 
{
    printf("- frame for x=%d, starting at %d\n",o->x,line);
}

void print_blit(object *o, int16_t x1, int16_t x2) 
{
    printf(" * blitting %d-%d\n",x1,x2);
}

void test(void)
{
    int posobj[][2] = {{0,10},{5,15},{25,30},{24,26},{12,32},{8,35},{0,640}}; // x1,x2 sorted by low to high, same y
    object objs[7];

    blitter_init();
    for (int z=0;z<7;z++) // XXX put with different negative y
    {
        objs[z] = (object) {
            .z=z, .x=posobj[z][0],.y=0,
            .h=10,.w=posobj[z][1]-posobj[z][0],
            .frame=print_frame, 
            .line=opaquerect_line, 
            .blit=print_blit 
        };

        blitter_insert(&objs[z]);
        printf("%d-%d\n",posobj[z][0],posobj[z][1]);
    }
    print_objects();
    blitter_frame();
    blitter_line();
}
*/
void blitter_line()
{
    // XXX optimize : combine three loops in one ?

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


    // reset opaque list to screen coordinates
    blt.opaque_list_nb = 2;
    blt.opaque_list[0] = (blit){.x1=INT16_MIN, .x2=0};
    blt.opaque_list[1] = (blit){.x1=LINE_LENGTH, .x2=INT16_MAX};
    //print_opaquelist();


    // now trigger each activelist, in Z descending order
    for (object *o=blt.activelist_head;o;o=o->activelist_next)
    {
        //printf("* line for object z= %d x=%d\n",o->z, o->x);
        o->line(o); // will call pre_draw and blit opaques
    }


    // finally, apply in ascending Z (bottom to front) the transparent blits
    transpblit t;
    for (int i=blt.transparent_stack_top-1;i>=0;i--)
    {
        t=blt.transparent_stack[i]; // shortcut
        t.o->blit(t.o, t.x1, t.x2);
    }

    // next line
    blt.current_line++;
}


void pre_draw (object *o, int16_t x1, int16_t x2,char is_opaque) 
{
    /*
        declare opaque for this line & call blitting'
        args : x1,x2,is_opaque  : declares span. '
        b : reference for callback to blit'

     there will always be a blit after x2, since screen boundaries is an "infinite" blit
     transparent = push a masked line to the draw FIFO, which will be emptied after the make opaque
    */

    //printf("-- predraw %d - %d (%s)\n",x1,x2,is_opaque?"opaque":"transp");

    // skip all past indices in the list. (ie indices completely before.)
    // XXX optimize : cache index ?
    int bltindex = 0; 
    while (x1>=blt.opaque_list[bltindex].x2) bltindex +=1;
    //printf("skipped to bltindex=%d\n",bltindex);

    while (1) 
    {
        //print_opaquelist();
        //printf("  bltidx : %d ; range : %d-%d\n",bltindex,x1,x2);

        // are we starting within a blit ? 
        if (blt.opaque_list[bltindex].x1<=x1)
        {
            x1 = blt.opaque_list[bltindex].x2; // skip to the blit end
            bltindex += 1; // next blit
        }

        // ok now we passed, we're sure to be after a blit and not inside it. 
        // we're also sure to have a preceding one

        // blit up to the next obstacle or the end


        if (x1>=x2) break; // already finished ? (includes end of line)


        // there is a next blit here since x1 would have been MAXINT if it was the last
        if (x2<blt.opaque_list[bltindex].x1)  // do we finish before or at the start of the next opaque ? if yes we can finish the blit here
        {
            //printf("finishing\n");
            // blit all ; blit start is end of preceding blit ?
            if (is_opaque) 
            { 
                o->blit(o,x1,x2); // blit effectively

                // mark opaque
                if (x1==blt.opaque_list[bltindex-1].x2) 
                {
                    // extend the current blit
                    blt.opaque_list[bltindex-1].x2=x2;
                }
                else 
                { 
                    // insert a new one up to end (if there is room)
                    if (blt.opaque_list_nb<MAX_BLITS)
                    {
                        // move the existing ones to the end, creating a hole
                        for (int i=blt.opaque_list_nb;i>=bltindex;i--)
                            blt.opaque_list[i+1]=blt.opaque_list[i];
                        blt.opaque_list_nb++;
                        // put the new one in the hole
                        blt.opaque_list[bltindex].x1=x1;
                        blt.opaque_list[bltindex].x2=x2;                    
                    }
                }
            } else { // transparent
                transparent_push(x1,x2,o);
            }
            break;
        } else { // not finishing
            if (is_opaque) 
            { 
                o->blit(o, x1,blt.opaque_list[bltindex].x1-1); // blit up to start of next
                // mark opaque : extend on the left the next one 
                // (we ARE in line with preceding here, so we extend)
                blt.opaque_list[bltindex].x1=x1;
            } else {
                transparent_push(x1,  blt.opaque_list[bltindex].x1, o);
            }
            x1 = blt.opaque_list[bltindex].x2; // skip current
            //bltindex += 1; // XXX not always, we're extending preceding.
        }
    } // break
    //printf("finally:");print_opaquelist();
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
