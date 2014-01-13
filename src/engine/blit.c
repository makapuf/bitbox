#include "blit.h"

// XXX Add some locking within frame ? 

#include <stdio.h> // printf
#include <string.h> //memcpy

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

void print_opaquelist(void) 
{
	for (int i=0;i<blt.opaque_list_nb;i++)
	{
		printf ("%d:%d, ",blt.opaque_list[i].x1, blt.opaque_list[i].x2);
	}
	// now transp as put

	printf("\n");
}


void print_objects()
{
	printf(" -- object list\n");
	object *o;
	for (int i=0;i<blt.nb_objects;i++)
	{
		o=blt.objects[i];
		if (o) {
			printf ("x=%d y=%d w=%d h=%d z=%d a=%d\n",o->x,o->y, o->w,o->h,o->z,o->a);
		} else {
			printf ("<empty>\n");
		}
	}
	printf(" --\n");
}


void blitter_init()
{	
	for (int i=0;i<MAX_OBJECTS;i++) blt.objects[i]=0; // empty objects array
	blt.nb_objects = 0;
}

int blitter_insert(object *o)
// return blitter index for new object, negative if none found
// append to end of list ; list ends up unsorted now
{
	if (blt.nb_objects<MAX_OBJECTS)
		blt.objects[blt.nb_objects++] = o;
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
static void blitter_sort_objects_y()
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
			{	// change head ?
				blt.activelist_head = o->activelist_next;
			} else {
				prev->activelist_next=o;
			}
		}
		prev=o;
	}

	// add new active objects
	while (blt.next_to_activate<blt.nb_objects && blt.current_line>=blt.objects[blt.next_to_activate]->y)
	{
		activelist_add(blt.objects[blt.next_to_activate]);
		blt.next_to_activate++;
	}

	// reset opaque list to screen coordinates
	blt.opaque_list_nb = 2;
	blt.opaque_list[0] = (blit){.x1=MININT, .x2=0};
	blt.opaque_list[1] = (blit){.x1=SCREEN_WIDTH, .x2=MAXINT};


	// now trigger each activelist, in Z descending order
	for (object *o=blt.activelist_head;o;o=o->activelist_next)
	{
		o->line(o); // will call pre_draw and blit opaques
	}

	// finally, apply in ascending Z (bottom to front) the transparent blits
	transpblit t;
	for (int i=blt.transparent_stack_top-1;i>=0;i--)
	{
		t=blt.transparent_stack[i]; // shortcut
		t.o->blit(t.o, t.x1, t.x2);
	}
}



static inline void transparent_push(int16_t x1, int16_t x2,object *o)
{
	if (x1<x2 && blt.transparent_stack_top<MAX_BLITS) 
	{
		blt.transparent_stack[blt.transparent_stack_top] = (transpblit){.x1=x1, .x2=x2, .o=o};
		blt.transparent_stack_top+=1;
	} 
}


void pre_draw (object *o, int16_t x1, int16_t x2,char is_opaque) 
{
	/*
	NOTES
	 there will always be a blit after x2, since screen boundaries is an "infinite" blit
	 transparent = push a masked line to the draw FIFO, which will be emptied after the make opaque
	*/

	//printf("\n-- blitting predraw  %d - %d (%s)\n",x1,x2,is_opaque?"opaque":"transp");

	// skip all past indices in the list. 
	// XXX optimize : cache index ?
	int bltindex = 0; 
	while (x1>=blt.opaque_list[bltindex].x2) bltindex +=1;
	//printf("skipped to bltindex=%d\n",bltindex);

	while (1) 
	{
		//print blt.opaque_list, 'idx:',bltindex,'range', x1, x2
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
		if (x2<=blt.opaque_list[bltindex].x1)  // do we finish before or at the start of the next opaque ? if yes we can finish the blit here
		{
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
						for (int i=bltindex;i<blt.opaque_list_nb;i++){							
							blt.opaque_list[i+1]=blt.opaque_list[i];
						}
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
		} else {
			if (is_opaque) 
			{ 
				o->blit(o, x1,blt.opaque_list[bltindex].x1); // blit up to start of next
				// mark opaque : extend on the left the next one 
				// (we ARE in line with preceding here, so we extend)
				blt.opaque_list[bltindex].x1=x1;
			} else {
				transparent_push(x1,  blt.opaque_list[bltindex].x1, o);
			}
			x1 = blt.opaque_list[bltindex].x2; // skip current
			bltindex += 1;
		}
	}
	print_opaquelist();
}





// --- implementations

void dummy_frame(object *o,int line) {}

void opaquerect_line (object *o)
{
	pre_draw (o, o->x,o->x+o->w,OPAQUE) ;
}
void transprect_line (object *o)
{
	pre_draw (o, o->x,o->x+o->w,TRANSPARENT);
}

void color_blit(object *o, int16_t x1, int16_t x2) 
{
	// memset of draw_buffer ...
}

/*  RLE Sprite 

 	a is the base pointer to uint16_t data[]  (reset value of b) 	  
 	b is a moving pointer within this data

	c is a temp data

 	data is made of N records of :  
    
    uint16_t blit (see after)
    uint16_t value[2](if blit.fill)
    uint16_t pixel_data[blit.nb] (if not blit.fill)


	note that a/b as pointers implies sizeof(void*) == sizeof (int)
	compile with -m32 if needed (needs gcc-multilib on x64)
 */

typedef struct __attribute__ ((__packed__))  {
	unsigned int skip:7; // in pixels
	unsigned int nb:7; // in pixels

	unsigned int fill:1; // 2-pixels fill ?
	unsigned int eol:1; // skip to next line
} Blit;

void rle_sprite_frame(object *o, int line)
{
	o->a=o->b;
	// XXX fast forward to line
}

void rle_sprite_line(object *o, int line)
{
	Blit blit;
	o->c=o->x; // save starting x of this blit
	do {
		blit = *((Blit*)o->b); // peek blit data
		o->c += blit.skip;
		// prepare blit
		pre_draw(o,o->c,o->c+blit.nb, OPAQUE); // this will call blit 
		o->c += blit.nb; // pixels
    } while (!blit.eol); 
}

extern uint16_t draw_buffer;

void rle_sprite_blit(object *o, int16_t x1, int16_t x2)
{
	Blit blit = *((Blit*)o->b); // read blit data
	o->b += 2;  // next 
	// COPY mode (not fill)
	// the current blit data is next. We wanted to blit blit.nb pixels from o->b at pixel o->c
	memcpy((void*)(draw_buffer + o->c),(void*) (o->b+x1-o->c), (x2-x1)*2); // verify ...
	o->b += blit.nb*2; // ptr of u16

	// XXX FILL MODE !
}