#include <kernel.h>
#include <stdint.h>
#include "fatfs/ff.h"

volatile int x,y;
// x and y  should be volatile since the vga thread must see the changes to x and y 
// which are runnin in the main thread 

#define POS_IMG (200+y) // pos of image 
#define IMGHEADER 0xfaceb0b0
typedef struct {
	uint32_t header;
	uint16_t x,y;
	uint16_t palette[256]; 
	uint8_t data[320*200];
	uint32_t footer;
} my_image;


my_image image1;
my_image image2 __attribute__ ((section (".ccm")));

my_image *im_current = &image1, *im_other = &image2;

FATFS fatfs;
FIL anim_file;
FRESULT file_opened;


void game_init() {
	audio_on=1;
	// open file & testif present & OK (dimensions, ...)

	file_opened = f_mount(&fatfs,"",1); //mount now
	if (file_opened==FR_OK)
		file_opened = f_open (&anim_file, "anim.bin",FA_READ); // open file		
}

void game_frame()
{
	UINT BytesRead;

	// receive events
    if (PRESSED(up) && y>-240) y--;
    if (PRESSED(down) && y<240) y++;
    if (PRESSED(left) && x>-320) x--;
    if (PRESSED(right) && x<320) x++;

    
    // at 20FPS = each 3 frames, read an image from disk
    if (file_opened==FR_OK && vga_frame%3 == 0 )
    {
	    // swap loading and displaying images
	    my_image *tmp;
	    tmp=im_other;
	    im_other=im_current; 
	    im_current=tmp;

	    // start loading frame from SD (XX check file)
		f_read (&anim_file, im_other, sizeof(my_image), &BytesRead);
		if (BytesRead != sizeof(my_image))
		{
	    	// if last_frame :rewind & retry	    	
			f_lseek (&anim_file,0);			
			f_read (&anim_file, im_other, sizeof(my_image), &BytesRead);
		}
    }
    
} 

void game_line()
// called from VGA kernel
{	
	int i=0;

	// display current frame at (0,y) if loaded
	if (im_current->header==IMGHEADER && vga_line > POS_IMG && vga_line<POS_IMG+im_current->y)
	{
		uint8_t *src = &(im_current->data[(vga_line-POS_IMG)*im_current->x]);
		uint16_t *dst = draw_buffer;

		for (i=0;i<im_current->x;i++)
			*dst++ = im_current->palette[*src++];
		// finish line		
	} 

	// clear the line / end of line with a repeating gradient
	unsigned int fr = vga_frame;
	for (;i<LINE_LENGTH;i++)
		draw_buffer[i]=(vga_line&0x1f)<<(5*(((fr+vga_line)/32)&3)); 


	// first oblique line (behind)
	for (int i=0;i<128;i++)
		draw_buffer[vga_line+i] = (i/8)<<4;

    // display gamepad state as an inverse video point
    if (vga_line == 200) {
    for (int i=0; i<16; i++)
      if (gamepad1 & (1 << i)) draw_buffer[320+i]^=0x7fff;
    }
    	
    if (vga_line==200+y)
	{
		draw_buffer[320+x]^=0x7fff;
	}
}

void game_snd_buffer(uint16_t *buffer, int len) 
/* generates a 500Hz sound alternating between left & right */
{
	for (int i=0;i<len;i++)
	{
		if ((i/64)&1) 
		{
			*buffer++ = (vga_frame/8)&1 ?  0x4000 : 0x0040;
		} else {
			*buffer++ = 0;
		}

	}
};
