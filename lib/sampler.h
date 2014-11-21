/* micro_sampler

plays a number of sounds, 
 - directly from data in memory 
 - from a raw file 


to use it, 
 - include the file (it will provide the sound callback to the kernel) to build
 - init the engine
 - 
 */
#ifndef NO_SDCARD
#include "fatfs/ff.h" // FIL
#endif 

#define MAX_VOICES 64


/* plays a sound once stored from data in memory. 

   data, data_len is an array of sound data as uint8_t
   vol_left and vol_right define the panning from 0 to 255 (as 1-256 / 256)

   returns a voice_id, useful for stopping it
   if no free voice is found, the sound is not played and a negative value is returned
   
   */
int play_mono_from_memory(const int8_t *data, int data_len, uint8_t vol_left, uint8_t vol_right);

#ifndef NO_SDCARD
/* plays a sound streamed from a file 
	the RAM buffer should be sufficient to allow skip free operation
	of course it shall not be shared between voices.
	
	The file should be already opened and seek'd to the right position

	vol_right and left are for panning in the global mix.

	returns a voice_id
   	if no free voice is found, the sound is not played and a negative value is returned
	
	*/
int play_mono_from_raw_file(FIL *f, int8_t *buffer, int buffer_len, uint8_t vol_left, uint8_t vol_right);
#endif 

// stop all samples from playing
void stop_all_samples();

// stop a given sample (other samples continue playing)
void stop_sample(int voice_id);
