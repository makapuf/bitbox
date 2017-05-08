#include <string.h>

#include <bitbox.h>
#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#define DATA_IMPLEMENTATION
#include "data.h"

#include "lib/mod/mod32.h"

int sample_in_tick;

void loadNextFile() {
	if (mod==(struct Mod*) data_sotb1_mod){
		load_mod((struct Mod*) data_cocopops_mod);
		set_led(0);
	} else {
		load_mod((struct Mod*) data_sotb1_mod);
		set_led(1);
	}

	message("Switched to mod, song name: [%s]\n",mod->name);
}


void game_init()
{
	loadNextFile();
}


void game_frame() {
	static int prev_but;
	// check button for next song ...
	if (!button_state() && prev_but){
		loadNextFile();
	} 

	prev_but=button_state();
}

void graph_line() {}

void game_snd_buffer(uint16_t *stream, int size)
{
	if (!player.numberOfChannels)
		return;

	for (int i=0;i<size; i++) {

		// song status / change updating		
		if (sample_in_tick++==player.samplesPerTick) /// ROW DELETE ME !
		{
			update_player();
			sample_in_tick=0;
		}
		
		stream[i] = gen_sample(stream); 
	}
} 