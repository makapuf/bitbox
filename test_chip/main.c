#include "bitbox.h"
#include "chiptune.h"

extern const struct ChipSong SONG;

void game_init() {
	chip_init(&SONG);
}

// ouch, that was hard !

void game_frame() {}
void graph_frame() {}
void graph_line() {}
