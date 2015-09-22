/* micro_sampler

plays a number of sounds, 
 - directly from data in memory 
 - from a raw file (i8 raw)


to use it, 
 - include the file (it will provide the sound callback to the kernel) to build
 - init the engine
 - 
 */

#include <stdint.h>

#define MAX_VOICES 16


/* plays a sound once stored from data in memory. 

   data, data_len is an array of sound data as int8_t
   vol_left and vol_right define the panning from 0 to 255. Setting volume to 0,0 frees the note.
   loop_pos is the position of the sample where to loop at once we reach position data_len. Set to -1 to play once.
   speed is the speed in samples per second relative to the BITBOX_SAMPLERATE (32000) *256, so 256 is 32k samples/s

   returns a voice_id - useful for stopping it, or setting volume by example
   if no free voice is found, the sound is not played and a negative value is returned
   
   */

int play_sample(const int8_t *data, int data_len, uint16_t speed, int loop_pos, uint8_t vol_left, uint8_t vol_right);

void set_vol(int voice_id, uint8_t vol_left, uint8_t vol_right);

// stop all samples from playing
void stop_all_samples( void );

// stop a given sample (other samples continue playing)
void stop_sample(int voice_id);

// reading and playing songs
struct NoteEvent {
	uint16_t tick; // as 96 PPQ since last one.
	int8_t note; // midi note. <0 are special ones.
	uint8_t vel; // velocity. set to zero for note off.
};

void play_track (int nb_events, int tempo, const struct NoteEvent *events, 
	const int8_t *sound_data, int sound_loop, int data_len, int c4freq);
