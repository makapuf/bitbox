#include "bitbox.h"
#include "lib/chiptune/player.h"

extern const struct ChipSong SONG;

void game_init() {
	chip_play(&SONG);
}

// ouch, that was hard !

void game_frame() {set_led(chip_song_playing());}
void graph_line() {}
