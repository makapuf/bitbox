#include <stdint.h>
#include "bitbox.h"

int song=3;
unsigned int t;

// bitbox port Makapuf 2015 - code GPL - songs by their respective author, idea and first implmentation by Viznut
// See : http://countercomplex.blogspot.fr/2011/10/algorithmic-symphonies-from-one-line-of.html <-- not one line of html :)
// also : http://pelulamu.net/countercomplex/music_formula_collection.txt

/*  yes, the whole melody is done with single line equations ...  */

void game_snd_buffer(uint16_t* buffer, int len) {
	uint8_t u=0,v;

	for (int i = 0; i < len; i++) {
		if (i%4 ==0) {
			t++; // sample rate = 8kHz
			switch(song) {
				case 0 : u=(t*(t>>8|t>>9)&46&t>>8)^((t&t>>13)|t>>6); break; // "lost in space" by xpansive
				case 1 : u=(t|(t>>9|t>>7))*t&(t>>11|t>>9); break; // by red-
				case 2 : u=(t*5&(t>>7))|(t*3&(t*4>>10)); break;
				case 3 :
					v=t>>(7-(t>>15))&-t>>(7-(t>>15)); // By droid
					u=v?t>>4|(t&(t>>5) / v) : 0; // guarding against divbyzero
					break;
				case 4 : u=t*(42&t>>10);break; // by .. many
				case 5 : u=(t>>(t>>5*(t>>13)%8))|(t>>4) ;break; // by The_Cjay
				case 6 : u=((1-(((t+10)>>((t>>9)&((t>>14))))&(t>>4&-2)))*2)*(((t>>10)^((t+((t>>6)&127))>>10))&1)*32+128;break; // by Ola
				case 7 : u=t*((t>>13)&31&t>>9)-((t>>7)&23&t>>3); break; // ack
				default : u=0;
			}
		}

		buffer[i] = u<<8 | u;
	}
}


// complex "game" indeed
void game_init() {}
void game_frame() {
	static int pb;
	int b=button_state();
	if (b&!pb) {
		song = (song +1 )%8;
		t=0;
		message("song %d\n",song);
	}
	pb=b;
}
void graph_line() {}
void graph_frame() {}
