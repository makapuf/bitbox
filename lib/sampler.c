/*
TODO : 
	- loop point, 
	- acceleration of mixing func
	- read from WAV (incl loop points?)

	- vol enveloppes ?
	- resampling (+ interpolation?)
	- note stealing
	- dither
	- synth voices ?

*/
#include <stdint.h>
#include <string.h> 

#include "bitbox.h"
#include "sampler.h"

#define abs(x) ((x)>0?(x):-(x))
struct Voice {
	int8_t *data; // RAM buffer for streamed, or direct data for mem-based. NULL if voice not used. Assumes MONO, provide two files/data for stereo
	int data_len;  
	int data_pos; // current position in data/buffer (not file)
	#ifndef NO_SDCARD
	FIL *file; // non-null for streamed file type voice, null means data sample
	#endif 	
	uint8_t vol_left, vol_right; // 0-255 per channel ? (as 1-256)
};

struct Sampler {
	struct Voice voices[MAX_VOICES];
};

// ----------------------------------------------------------
static struct Sampler s;
// ----------------------------------------------------------

static int find_free_voice()
{
	for (int i=0;i<MAX_VOICES;i++)
		if (!s.voices[i].data) return i;
	return -1;
}

int play_mono_from_memory(const int8_t *data, int data_len, uint8_t vol_left, uint8_t vol_right)
{
	int idx=find_free_voice();
	if (idx>=0) {
		struct Voice *v = &s.voices[idx];
		v->data=(int8_t*) data; // non-const
		v->data_len=data_len;
		v->vol_left=vol_left;
		v->vol_right=vol_right;
		
		#ifndef NO_SDCARD
		v->file=0;
		#endif

		// rewind
		v->data_pos=0; 
	}
	return idx;
}

#ifndef NO_SDCARD
// buffer must be at least one internal buffer size !
int play_mono_from_raw_file(FIL *f, int8_t *buffer, int buffer_len, uint8_t vol_left, uint8_t vol_right)
{
	int idx=find_free_voice();
	if (idx>=0) {
		struct Voice *v = &s.voices[idx];
		v->file=f;
		v->data=buffer;
		v->data_len=buffer_len;
		v->vol_left=vol_left;
		v->vol_right=vol_right;

		// rewind
		v->data_pos=0; 
	}
	return idx;
}
#endif 

void stop_sample(int sample_id)
{
	s.voices[sample_id].data=0;
}

void stop_all_samples()
{
	for (int sample_id=0;sample_id<MAX_VOICES;sample_id++)
		s.voices[sample_id].data=0;
}

void game_snd_buffer(uint16_t *buffer, int len)
{
	// MIX voices into buffer.

	struct Voice *v;
	uint8_t *buffer8 = (uint8_t*)buffer;

	for (int i=0;i<=len*2;i++)
		buffer8[i]=128;


	for (int vi=0;vi<MAX_VOICES;vi++) 	
	{
		v=&s.voices[vi];
		if (!v->data) continue; // skip
		int nb = v->data_len-v->data_pos; // number of samples available in memory
		if (len<nb) nb=len; 

		// mixing available data to buffer
		for (int i=0;i<=nb;i++) {
			// use assembly / SIMD instrs !

			int8_t smp = v->data[v->data_pos+i];
			int8_t a = (smp*v->vol_left)>>8;
			int8_t b = (smp*v->vol_right)>>8;
			buffer8[i*2] +=  a;
			buffer8[i*2+1] +=  b;
		};


		// end of buffer, whichever it is
		v->data_pos+=nb;

		// end of memory / filebuffer ?
		if (v->data_pos==v->data_len) {
			
			#ifndef NO_SDCARD
			if (v->file) {
				// end of filebuffer, load next chunk 
				size_t bytes_read;
				f_read(v->file, v->data, v->data_len,&bytes_read); // XXX ASYNC ?
				// finish filling buffer
				for (int i=0;i<=len-nb;i++)
					buffer[i] += ((v->data[v->data_pos+i]*v->vol_left)>>8)<<8 | (v->data[v->data_pos+i]*v->vol_right) >>8;
				v->data_pos=len-nb; // rewind buffer ; filebuffer len is at least one internal buffer size.
			} else 
			#endif 

			{ // memory-based : end of sample
				v->data=0; // free voice
			}
		} 
	}
} 
