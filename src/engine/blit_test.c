#include <stdio.h>
#include "blit.h"


void print_frame(object *o,int line) 
{
	printf("- frame for x=%d, starting at %d\n",o->x,line);
}

void print_blit(object *o, int16_t x1, int16_t x2) 
{
	printf(" *** blitting %d-%d\n",x1,x2);
}

void print_blit_tr(object *o, int16_t x1, int16_t x2) 
{
	printf(" *** transp blitting %d-%d\n",x1,x2);
}

extern void print_objects();


int main(void)
{
	int posobj[][2] = {{0,10},{5,15},{25,30},{24,26},{12,32},{8,35},{0,640}}; // x1,x2 sorted by low to high, same y
	object objs[9];

	blitter_init();
	for (int z=0;z<7;z++) // XXX put with different negative y
	{
		objs[z] = (object) {
			.x=posobj[z][0],
			.y=-z,
			.z=2*z, 
			.w=posobj[z][1]-posobj[z][0],
			.h=10,
			.frame=print_frame, 
			.line=opaquerect_line, 
			.blit=print_blit 
		};

		blitter_insert(&objs[z]);
		printf("%d-%d\n",posobj[z][0],posobj[z][1]);
	}
	object t1 = (object) {
			.x=-50,.y=0,.z=9, .w=400,.h=10,
			.frame=print_frame, .line=transprect_line, .blit=print_blit_tr 
		};
	blitter_insert(&t1);
	object t2 = (object) {
			.x=-50,.y=0,.z=11, .w=640+50,.h=10,
			.frame=print_frame, .line=transprect_line, .blit=print_blit_tr 
		};
	blitter_insert(&t2);


	print_objects();
	blitter_frame();
	blitter_line();

}