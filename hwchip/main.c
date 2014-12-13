#include "bitbox.h"
#include "player.h"

#include <string.h>

void game_frame() {
	if (vga_frame == 10) {
		ply_init();
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
