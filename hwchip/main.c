#include "bitbox.h"
#include "player.h"

#include <string.h>

extern const unsigned char songdata[];

void game_frame() {
	// Apparently starting the song immediately at boot cuts part of the first
	// pattern out. So we delay it to the 10th VGA frame and then things run
	// stable. This is a bit unexpected...
	if (vga_frame == 10) {
		ply_init(songdata);
		audio_on=1;
	}

	if (vga_frame > 10)
		ply_update();
}

void game_init() {
}

void game_line() {};
void graph_frame() {}
void graph_line() {
}
