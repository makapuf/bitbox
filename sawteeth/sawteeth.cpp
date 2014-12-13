/* Sawteeth music player for BitBox
 * Copyright 2014, Adrien Destugues <pulkomandy@gmail.com>
 * This program is distributed under the terms of the MIT license.
 */

#include <stdint.h>
#include "headers/txt.h"
#include "headers/stSong.h"

extern const unsigned char rawData[];
extern unsigned int rawData_len;
static stSong* song;
static float tmp[1024];

extern "C" {
	#include "bitbox.h"

	void game_init() {
		audio_on = 1;
		txt songData((char*)rawData, rawData_len);
		song = new stSong(songData);
	};
	void game_frame() {};
	void game_line() {};

	void game_snd_buffer(uint16_t* buffer, int len) {
		song->Play(tmp, len);
		// downmix to uint8
		uint8_t* buf = (uint8_t*)buffer;
		for (int i = 0; i < len * 2; i++) {
			buf[i] = (uint8_t)(tmp[i] + 127);
		}
	};

	void graph_frame() {}
	void graph_line() {}
}

#ifndef EMULATOR

void* operator new(size_t size) {
	return malloc(size);
}

void operator delete(void* p) {
	free(p);
}

void* operator new[](size_t size) {
	return malloc(size);
}

#endif
