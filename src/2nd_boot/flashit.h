/* flashit.h : aynchronous writing to flash fro ma SD card */
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
