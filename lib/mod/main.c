#include <string.h>

#include <bitbox.h>
#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#define DATA_IMPLEMENTATION
#include "data.h"

#include "lib/mod/mod32.h"


void loadNextFile() 
{
	static int nb=0;
	if (nb){
		load_mod(data_cocopops_mod);
	} else {
		load_mod(data_sotb1_mod);	
	}
	
	nb = 1-nb;
	set_led(nb);

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


