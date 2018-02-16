/*
    Decoding BTC4 data in file to ram
    file are just concatenated btc frame files
*/

#include <stdint.h>
#include <bitbox.h>
#include <string.h>
#include "fatfs/ff.h"

#include "lib/blitter/blitter.h"
#ifndef EMULATOR
    #include "stm32f4xx.h"
#endif

#define VIDEO_FILENAME "video.bin"
#define MAX_BUFSIZE 58120 // leaves a little room for other vars (not in CCM)

// Dual buffer ; should be sufficient for 640x360
uint32_t buffer1[MAX_BUFSIZE/4];
uint32_t buffer2[MAX_BUFSIZE/4];

uint32_t *buffer_disp=buffer1;
uint32_t *buffer_load=buffer2;

object btc,black;
FATFS fatfs;
FIL video_file;
FRESULT result;

/* faire un objet video (passer dans le engine ensuite) , loop ?
video_init(file, u32 *buffer1, u32* buffer2, resize_min)
{
	a=&file
	read header
	load w,h (once) + other data
	initialize all, resize if needed
	decide how much to load each frame
	...
	check all is displayed
}

video_frame()
{

	exhange frames,
	check if even or odd frame

}

video_setframe() : file.seek(calculer), attendre frame paire

*/


void game_init()
{
    size_t bytes_read;

	result = f_mount(&fatfs,"",1); //mount now
	if (result==FR_OK) result = f_open (&video_file,VIDEO_FILENAME,FA_READ); // open file

	if (result==FR_OK) message ("ok, mounted & file opened ok\n");
	// dummy, empty frame
	// XXX crash of not the exact size of the movie ?? load a little header

	result = f_read (&video_file,buffer_load,512, &bytes_read);
	f_lseek(&video_file,0);

	if (buffer_load[0]<=320)
    	btc4_2x_insert(&btc, buffer_disp,0,0,0);
    else
    	btc4_insert(&btc, buffer_disp,0,0,0);

    rect_insert(&black, 0, 0, 640, 10,0, RGB(0,0,0)); // black bg after
}

volatile int t_tot; // for all frames

void game_frame()
{
	size_t bytes_read;
	unsigned int bytes_to_read=512;
	static uint32_t *dst;

	// every 2 frame  (30fps fixed)
	if (vga_frame%2==0) {
		// exchange frame buffers
		uint32_t *tmp = (uint32_t *)buffer_load;
		buffer_load=buffer_disp;
		buffer_disp=tmp;

		// setup new buffer for display
		btc.w = buffer_disp[0]*2;
		btc.h = buffer_disp[1]*2;

		btc.x = (640-btc.w)/2;
		btc.y = (480-btc.h)/2;

		btc.data = buffer_disp+2;
		black.y=240+btc.h/2;

		// / 4 because of 2x video
		bytes_to_read = btc.w*btc.h/4/4+512+8; // based on the PRECEDING size (any size change will crash, so no need to ba able to change)
		bytes_to_read = 512*((bytes_to_read+511)/512); // pad to next 512 bytes
		// should depend of frame size (which should be constant)
		bytes_to_read -= 8192;
		dst = buffer_load;

	} else {
		bytes_to_read = 8192;
		// do not modify dst, it will continue from where it was
	}

	// load next frame or second half
	if (result==FR_OK) { // sometimes KO but needs mounted
		// read all file, might not read all.
		#ifndef EMULATOR
		int t0 = DWT->CYCCNT;
		#endif
		result = f_read (&video_file,dst,bytes_to_read, &bytes_read); // errors because of data timeout  ... to be checkeds
		#ifndef EMULATOR
		t0 = DWT->CYCCNT-t0;
		t_tot += t0;
		#endif
		dst += bytes_read/4;

		// eof : loop it.
		if (result==FR_OK && bytes_read<bytes_to_read) {
			result = f_lseek(&video_file,0);
			message ("reloaded file\n");
		}
	}

}




void game_snd_buffer(uint16_t *buffer, int len) {
	// FIXME sound
}
