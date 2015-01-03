#include "bitbox.h"
#include "player.h"

#include <string.h>

void game_frame() {
	if (vga_frame == 10) {
		ply_init();
	}

	if (vga_frame > 10)
		ply_update();
}

void game_init() {
}

void graph_frame() {}
void graph_line() {
}
