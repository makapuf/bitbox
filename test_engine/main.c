#include <math.h>
#include <bitbox.h>
#include <blitter.h>

// #define SPRITE mechant3_16_spr
#define SPRITE img0001_p16_spr



extern const char SPRITE[];
extern const uint16_t bg_tset[];
extern uint32_t bg_bg_tmap[];

// x and y  should be volatile since the vga thread must see the changes to x and y 
// which are runnin in the main thread 
#define NB 4
#define MOVE_SQUARE 0
#define MOVE_BALLS 1
#define ROTATE_BALLS 0

object *ball[NB], *bg, *square;
int vx[NB],vy[NB];
int ini_vy []= {-4, 3, 1,-6, 4,-7, 3,-5};
int ini_vx []= {-2, 7, 1, 2, 4,-4, 3, 1 ,3};
int ini_vz []= {-3,-5,-2,-2,-2, 3, 4, 5};


void game_init() {
	blitter_init();

	bg=tilemap_new(bg_tset,640,65536,bg_bg_tmap); 
	bg->x=bg->y=0;
	bg->z = 200;
	
	if (MOVE_SQUARE)
		square=rect_new (10,10,100,100,150, RGB(0xff,0,0));
	
	for (int i=0;i<NB;i++)
	{
		vx[i]=ini_vx[i%8];
		vy[i]=ini_vy[i%9];
		ball[i] = sprite_new((uint32_t *)&SPRITE);
		ball[i]->x = i*(640-ball[i]->w)/(NB+1);
		ball[i]->y = 0;
		ball[i]->z = i;
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
		    if (ball[i]->x + vx[i] >= (640-ball[i]->w) || ball[i]->x <0 )
		      	vx[i] = -vx[i];
		    
		    if (ball[i]->y +vy[i] >= 480-ball[i]->h)
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
    blitter_frame();
} 

void game_line()
{	
	blitter_line();
}

void game_snd_buffer(uint16_t *buffer, int len) {}