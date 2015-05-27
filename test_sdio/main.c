#include <bitbox.h>
#include <stdint.h>
#include <string.h> // memcpy, even if memcpy reaallly is improved in future version of newlib
#include "fatfs/ff.h"


#define IMGHEADER 0xb71b
typedef struct {
	uint16_t header;
	uint16_t w,h;
	uint16_t data[]; // depends on the video !
} my_image;

const uint16_t result_colors[] = { 
	31<<10,            // red : init
	31<<10|31<<5,      // yellow : ok mount
	       31<<5,      // green : file opened
	       31<<5 | 31, // cyan : read data
	               31 , // blue : header OK
	31<<10       | 31  // pink : last header OK
};

uint16_t data[32768];
my_image *img=(my_image *)&data[0];

FATFS fatfs;
FIL img_file;
FRESULT result;

volatile int state=0; // from 0 to 5 according to step

void game_init() {
	// open file , load raw u16 frame from frame.bin
	state=0;
	result = f_mount(&fatfs,"",1); //mount now
	if (result==FR_OK)
	{
		state=1; // mount OK
		
	}
}

void game_frame()
{
	UINT BytesRead;
	uint8_t msg2[2]; // 2 bytes to read
	unsigned int value;


	// blink "result" times ...
	if (state==1)
	{
		result = f_open (&img_file,"image.bin",FA_READ); // open file		

		if (result==FR_OK)
		{
			state=2; // opened
			// read all file, might not read all.
			result = f_read (&img_file, &data, 65536, &BytesRead);
			if (result) state=3; // read
			if (img->header == IMGHEADER) state=4; // header OK
			if (img->data[img->w*img->h] == IMGHEADER) state=5; // last header OK (error?)
			f_close(&img_file);
		} 

		if (result==FR_OK) {
			// open file or create it & update it
			result = f_open (&img_file,"hello.txt",FA_READ | FA_WRITE | FA_OPEN_ALWAYS); 
			if (result==FR_OK) {
				// try to read or 
				if (f_read(&img_file, &msg2, 2, &BytesRead) == FR_OK && BytesRead == 2) {
					value=(msg2[0]-'0')*10+(msg2[1]-'0') + 1;
				} else {
					value=0;
				}
				msg2[0]=(value/10)+'0';
				msg2[1]=(value%10)+'0';
				
				f_lseek(&img_file,0);
				f_write (&img_file,  &msg2, 2, &BytesRead); // dont check result
				f_close (&img_file);
			}
		}
	}
} 
void graph_frame() {}
void graph_line()
// called from VGA kernel
{	
	// display image at (0,0) if loaded
	if (state>=4 && vga_line<img->h)
	{
		memcpy(draw_buffer, &img->data[vga_line*img->w],img->w*2);
	} 
	// black line after
	if (vga_line/2==img->h/2) memset(draw_buffer,0,640*2);

	// fills color code in lower part of screen
	if (vga_line>360 && vga_line<420)
	{
		for (int i=0;i<640;i++) draw_buffer[i]=result_colors[state];
	}
	if (vga_line/2==420/2) memset(draw_buffer,0,640*2);

}
