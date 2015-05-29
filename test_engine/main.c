#include <math.h>
#include <bitbox.h>
#include <blitter.h>

#include "bg.h"

#define SPRITE ball_small_spr
#define SPRITE_BIG ball_spr

extern const char SPRITE[];
extern const char SPRITE_BIG[];

// x and y  should be volatile since the vga thread must see the changes to x and y 
// which are runnin in the main thread 
#define NB_small 10
#define NB_big 3
#define NB (NB_small+NB_big)
#define MOVE_SQUARE 0
#define MOVE_BALLS 1
#define ROTATE_BALLS 1
#define TILED_BG 1

object *ball[NB], *bg, *square;
int vx[NB],vy[NB];
const int ini_vy []= {-4, 3, 1,-6, 4,-7, 3,-5};
const int ini_vx []= {-2, 7, 1, 2, 4,-4, 3, 1 ,3};
const int ini_vz []= {-3,-5,-2,-2,-2, 3, 4, 5};
const int ini_y  []= {-50,50,100,200,0,60,40,30};


void game_init() {
	blitter_init();

	if (TILED_BG)
		bg= tilemap_new (bg_tset, 64*16, 65535,bg_header,bg_tmap);
	else
		bg= rect_new (0,0,VGA_H_PIXELS, VGA_H_PIXELS*3,200, RGB(100,100,100));

	bg->z=200;
	
	if (MOVE_SQUARE)
		square=rect_new (10,10,100,100,150, RGB(0xff,0,0));
	
	for (int i=0;i<NB_small;i++) {
		vx[i]=ini_vx[i%8];
		vy[i]=ini_vy[i%9];
		ball[i] = sprite_new((uint32_t *)&SPRITE, 0,ini_y[i%7], i);
		ball[i]->x = i*(VGA_H_PIXELS-ball[i]->w)/(NB_small+1); // fix X after the fact
	}

	for (int i=0;i<NB_big;i++) {
		vx[i]=ini_vx[i%8];
		vy[i]=ini_vy[i%9]+10;
		ball[NB_small+i] = sprite_new((uint32_t *)&SPRITE_BIG, 0,ini_y[i%7], i);
		ball[NB_small+i]->x = i*(VGA_H_PIXELS-ball[NB_small+i]->w)/(NB_big+1); // fix X after the fact
	}
}


void game_frame()
{
	// move logo
	if (MOVE_SQUARE)
	{
		square->x = 60+40*cos(vga_frame/10.);
		square->y = 60+40*sin(vga_frame/10.);		
	}

	if (MOVE_BALLS)
	{
	    for (int i=0;i<NB;i++)
	    {	
		    if (ball[i]->x + vx[i] >= (VGA_H_PIXELS-ball[i]->w) || ball[i]->x <0 )
		      	vx[i] = -vx[i];
		    
		    if ((ball[i]->y + vy[i]) >= VGA_H_PIXELS-(int32_t)ball[i]->h )
		    	vy[i] = -vy[i]+1;


		    ball[i]->x += vx[i];
		    ball[i]->y += vy[i];
		    vy[i]+=1;
	    }
	}

	if (ROTATE_BALLS)
		for (int i=0;i<NB;i++)
		{	
			ball[i]->fr = (vga_frame*ini_vz[i%8]/4)%8;
		}
    
    uint32_t x = ((vga_frame%64)-32);
    bg->y = -x*x;
} 

