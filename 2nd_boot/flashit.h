/*

	Async flash : asychronously write a uSD file to flash memory.
	Assumes we wan write WORD by WORD (power is sufficient)

	Usage : 
		- flash_init
		- flash_start_write : call to launch a write of an open file to flash mem.
		- flash_frame : call each frame. Can be long, do whatever is needed before.
		- char flash_message[] : progress / error message 
	
	Statically allocs a buffer of memory (8k)

*/
#include "fatfs/ff.h"

void flash_init(); 

// starts a new flash write. Preceding must be finished. returns 0 if failed, 1 if OK
int flash_start_write(FIL *file); 

// true if no operation is taking place.
int flash_done(); 

/* to be called each frame.  
 will take as much time as needed for reading 
 a buffer or reach line X, leave it some time
*/

void flash_frame();

extern char flash_message[32];
