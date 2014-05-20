/*  
    Decoding BTC4 data in file to ram
    file are just concatenated btc frame files
*/

#include <stdint.h>
#include <kernel.h>
#include <string.h>
#include "fatfs/ff.h"

#include "blitter.h"

#define VIDEO_FILENAME "video.bin"
#define MAX_BUFSIZE 58120 // leaves a little room for other vars (not in CCM)
#define SKIPPED_FRAMES 2
    
// Dual buffer ; should be sufficient for 640x360
uint32_t buffer1[MAX_BUFSIZE/4];
uint32_t buffer2[MAX_BUFSIZE/4];

uint32_t *buffer_disp=buffer1;
uint32_t *buffer_load=buffer2;

object *btc,*black;
FATFS fatfs;
FIL video_file;
FRESULT result;

void game_init() 
{
    blitter_init();
    btc=btc4_new(buffer_disp,0,0,0);
    unsigned int bytes_read;

	result = f_mount(&fatfs,"",1); //mount now
	if (result==FR_OK) result = f_open (&video_file,VIDEO_FILENAME,FA_READ); // open file	

	if (result==FR_OK) message ("ok, mounted & file opened ok\n");	
	// dummy, empty frame
	// XXX crash of not the exact size of the movie ?? load a little header
	
	result = f_read (&video_file,buffer_load,16, &bytes_read); 
	f_lseek(&video_file,0);

	//buffer_load[0]=640;	buffer_load[1]=360;
	//buffer_load[0]=424;	buffer_load[1]=240;
    black=rect_new(0, 0, 640, 480,0, RGB(0,0,1)); // black bg after		
}

void game_frame() 
{
	unsigned int bytes_read;
	unsigned int bytes_to_read;

	// every X frame 
	if (vga_frame%SKIPPED_FRAMES==0) {
		// exchange frame buffers
		uint32_t *tmp = (uint32_t *)buffer_load; 
		buffer_load=buffer_disp; 
		buffer_disp=tmp;

		// setup new buffer for display
		btc->w = buffer_disp[0];
		btc->h = buffer_disp[1];

		btc->x = (640-btc->w)/2;
		btc->y = (480-btc->h)/2;

		btc->data = buffer_disp+2; 
		black->y=btc->h;

		// load next frame
		if (result==FR_OK) { // sometimes KO but needs mounted
			// read all file, might not read all.
			bytes_to_read = btc->w*btc->h/4+512+8; // based on the PRECEDING size (any size change will have one bad frame..)
			bytes_to_read = 512*((bytes_to_read+511)/512); // pad to next 512 bytes 
			result = f_read (&video_file,buffer_load,bytes_to_read, &bytes_read); // errors because of data timeout  ... to be checkeds
			// eof : loop it.
			
			if (result==FR_OK && bytes_read<bytes_to_read) {
				result = f_lseek(&video_file,0);
				message ("reloaded file\n");	
			}
		}
	}

    blitter_frame();
} 

void game_line() 
// simple default implementation
{
    blitter_line();
} 


void game_snd_buffer(uint16_t *buffer, int len) {}