#include "bitbox.h"
#include "chiptune.h"

extern const struct ChipSong bdash_chipsong;

void game_init() {
	chip_init(&bdash_chipsong);
}

// ouch, that was hard !

void game_frame() {}
void graph_frame() {}
void graph_line() {}
