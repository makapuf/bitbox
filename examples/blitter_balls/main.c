#include <math.h>
#include <bitbox.h>
#include "lib/blitter/blitter.h"

#define DATA_IMPLEMENTATION
#include "data.h"

extern const char ball_small_spr[];
extern const char bg_tset[];
extern const char bg_map[];

#if NB_big!=0
extern const char ball_spr[];
#endif

/* todo :
tests :
generate a droite et delete a gauche (genre de scroll) - pas de leaks
test bump beyond screen left& right - clipping
*/

//#define NB_small 40 - defined in makefile
//#define NB_big 3

#define NB (NB_small+NB_big)
#define MOVE_BALLS 1
#define ROTATE_BALLS 1
#define TILED_BG 1

object ball[NB], bg;

int vx[NB],vy[NB];
const int ini_vy [8]= {-4, 3, 1,-6, 4,-7, 3,-5};
const int ini_vx [9]= {-2, 7, 1, 2, 4,-4, 3, 1 ,3};
const int ini_vz [8]= {-3,-5,-2,-2,-2, 3, 4, 5};
const int ini_y  [8]= {0,-50,100,-100,0,260,140,30};

void game_init()
{
	if (TILED_BG) {
		tilemap_init (&bg, &data_bg_tset[4], 0, 0,TMAP_HEADER(64,64,TSET_16,TMAP_U8),&data_bg_map[8]);
		bg.h=65535; // special : looped
	} else {
		rect_init(&bg, VGA_H_PIXELS, VGA_H_PIXELS*3, RGB(100,100,100));
	}
	blitter_insert(&bg, 0,0,200);

	for (int i=0;i<NB_small;i++) {
		vx[i]=ini_vx[i%9];
		vy[i]=ini_vy[i%7];
		sprite3_load(&ball[i], &data_ball_small_spr);
		blitter_insert(&ball[i], 0,ini_y[i%8], i);

		ball[i].x = i*(VGA_H_PIXELS-ball[i].w)/(NB_small+1); // fix X after the fact
		if (i%4==0) sprite3_set_solid(&ball[i],RGB(255,0,0));
		if (i%8==1) sprite3_set_solid(&ball[i],RGB(255,255,0));
		if (i%8==0) sprite3_toggle2X(&ball[i]);
	}
#if NB_big != 0
	for (int i=NB_small;i<NB;i++) {
		vx[i]=ini_vx[i%9];
		vy[i]=ini_vy[i%7]+10;
		sprite3_load(&ball[i], &data_ball_spr);
		blitter_insert(&ball[i], 0,200+ini_y[i%8], 0);
		ball[i].x = (i-NB_small)*(VGA_H_PIXELS-ball[i].w)/(NB_big+1); // fix X after the fact
		if (i==NB_small+2) {
			sprite3_toggle2X(&ball[i]); // huge one
			ball[i].y = 0; // ensure does not touch bottom
		}
	}
#endif
}


void game_frame()
{
	if (MOVE_BALLS)	{
	    for (int i=0;i<NB;i++) {

		    if (ball[i].x + vx[i] >= (VGA_H_PIXELS-ball[i].w) || ball[i].x <0 )
		      	vx[i] = -vx[i];

		    if ((ball[i].y + vy[i]) > VGA_V_PIXELS-(int32_t)ball[i].h )
		    	vy[i] = -vy[i]+1;


		    ball[i].x += vx[i];
		    ball[i].y += vy[i];
		    vy[i]+=1;
	    }
	}

	if (ROTATE_BALLS) {
		for (int i=0;i<NB;i++) {
			ball[i].fr = (vga_frame*ini_vz[i%8]/4)%8;
		}
	}

    uint32_t x = ((vga_frame%64)-32);
    bg.y = -x*x;
}



void bitbox_main(void)
{
	game_init();
	while (1) {
		game_frame();
		wait_vsync(1);
	}
}
