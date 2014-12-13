/* Simple soundengine for the BitBox
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project
 *
 * This main file is a player for the packed music format used in the original
 * "hardware chiptune"
 * http://www.linusakesson.net/hardware/chiptune.php
 * There is a tracker for this but the format here is slightly different (mostly
 * because of the different replay rate - 31KHz instead of 16KHz).
 *
 * Because of this the sound in the tracker will be a bit different, but it can
 * easily be tweaked. This file looks complex, because it handles most of the
 * song format decoding (with a weird packed binary scheme to save space) and
 * was designed with an AVR8 in mind. Maybe the code could be made simpler.
 */

#include "engine.h"
#include "exported.h"

#define TRACKLEN 32

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef short s16;
typedef unsigned int u32;

u8 trackwait;
u8 trackpos;
u8 playsong;
u8 songpos;

static const u16 freqtable[] = {
	0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165, 0x017a, 0x0191, 0x01a9,
	0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259, 0x027d, 0x02a3, 0x02cb,
	0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3, 0x042f, 0x046f, 0x04b2,
	0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5, 0x070a, 0x0775, 0x07e6,
	0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c, 0x0bd6, 0x0c8b, 0x0d4a,
	0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb, 0x13e9, 0x1518, 0x1659,
	0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b, 0x217c, 0x237a, 0x2596,
	0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528, 0x3851, 0x3bab, 0x3f37,
	0x42f9, 0x46f5, 0x4b2d, 0x4fa6, 0x5462, 0x5967, 0x5eb7, 0x6459, 0x6a51,
	0x70a3, 0x7756, 0x7e6f
};

static const s8 sinetable[] = {
	0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
	127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
	0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
	-125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
	-71, -60, -49, -37, -25, -12
};

static const u8 validcmds[] = "0dfijlmtvw~+=";

struct trackline {
	u8	note;
	u8	instr;
	u8	cmd[2];
	u8	param[2];
};

struct track {
	struct trackline	line[TRACKLEN];
};

struct unpacker {
	u16	nextbyte;
	u8	buffer;
	u8	bits;
};

struct channel {
	struct unpacker		trackup;
	u8			tnum;
	s8			transp;
	u8			tnote;
	u8			lastinstr;
	u8			inum;
	u16			iptr;
	u8			iwait;
	u8			inote;
	s8			bendd;
	s16			bend;
	s8			volumed;
	s16			dutyd;
	u8			vdepth;
	u8			vrate;
	u8			vpos;
	s16			inertia;
	u16			slur;
} channel[4];

u16 resources[16 + MAXTRACK];

struct unpacker songup;

static inline u8 readsongbyte(u16 offset) {
	extern const u8 songdata[];
	return songdata[offset];
}

static void initup(struct unpacker *up, u16 offset) {
	up->nextbyte = offset;
	up->bits = 0;
}

static u8 readbit(struct unpacker *up) {
	u8 val;

	if(!up->bits) {
		up->buffer = readsongbyte(up->nextbyte++);
		up->bits = 8;
	}

	up->bits--;
	val = up->buffer & 1;
	up->buffer >>= 1;

	return val;
}

static u16 readchunk(struct unpacker *up, u8 n) {
	u16 val = 0;
	u8 i;

	for(i = 0; i < n; i++) {
		if(readbit(up)) {
			val |= (1 << i);
		}
	}

	return val;
}

static inline void readinstr(u8 num, u8 pos, u8 *dest) {
	dest[0] = readsongbyte(resources[num] + 2 * pos + 0);
	dest[1] = readsongbyte(resources[num] + 2 * pos + 1);
}

static void runcmd(u8 ch, u8 cmd, u8 param) {
	switch(cmd) {
		case 0: // 0 = note off
			channel[ch].inum = 0;
			break;
		case 1: // d = duty cycle
			osc[ch].duty = param << 8;
			break;
		case 2: // f = volume slide
			channel[ch].volumed = param;
			break;
		case 3: // i = inertia (auto note slides)
			channel[ch].inertia = param << 1;
			break;
		case 4: // j = instrument jump
			channel[ch].iptr = param;
			break;
		case 5: // l = slide
			channel[ch].bendd = param;
			break;
		case 6: // m = duty variation
			channel[ch].dutyd = param << 6;
			break;
		case 7: // t = timing
			channel[ch].iwait = param;
			break;
		case 8: // v = volume
			osc[ch].volume = param;
			break;
		case 9: // w = select waveform
			osc[ch].waveform = param;
			break;
		case 11: // + = set relative note
			channel[ch].inote = param + channel[ch].tnote - 12 * 4;
			break;
		case 12: // = = set absolute note
			channel[ch].inote = param;
			break;
		case 10: // ~ = vibrato
			if(channel[ch].vdepth != (param >> 4)) {
				channel[ch].vpos = 0;
			}
			channel[ch].vdepth = param >> 4;
			channel[ch].vrate = param & 15;
			break;
	}
}

void ply_init() {
	u8 i;
	struct unpacker up;

	trackwait = 0;
	trackpos = 0;
	playsong = 1;
	songpos = 0;

	osc[0].volume = 0;
	channel[0].inum = 0;
	osc[1].volume = 0;
	channel[1].inum = 0;
	osc[2].volume = 0;
	channel[2].inum = 0;
	osc[3].volume = 0;
	channel[3].inum = 0;

	initup(&up, 0);
	for(i = 0; i < 16 + MAXTRACK; i++) {
		resources[i] = readchunk(&up, 13);
	}

	initup(&songup, resources[0]);
}


void ply_update()
{
	u8 ch;

	if(trackwait) {
		trackwait--;
	} else {
		trackwait = 4;

		if(!trackpos) {
			if(playsong) {
				if(songpos >= SONGLEN) {
					playsong = 0;
				} else {
					for(ch = 0; ch < 4; ch++) {
						u8 gottransp;
						u8 transp;

						gottransp = readchunk(&songup, 1);
						channel[ch].tnum = readchunk(&songup, 6);
						if(gottransp) {
							transp = readchunk(&songup, 4);
							if(transp & 0x8) transp |= 0xf0;
						} else {
							transp = 0;
						}
						channel[ch].transp = (s8) transp;
						if(channel[ch].tnum) {
							initup(&channel[ch].trackup, resources[16 + channel[ch].tnum - 1]);
						}
					}
					songpos++;
				}
			}
		}

		if(playsong) {
			for(ch = 0; ch < 4; ch++) {
				if(channel[ch].tnum) {
					u8 note, instr, cmd, param;
					u8 fields;

					fields = readchunk(&channel[ch].trackup, 3);
					note = 0;
					instr = 0;
					cmd = 0;
					param = 0;
					if(fields & 1) note = readchunk(&channel[ch].trackup, 7);
					if(fields & 2) instr = readchunk(&channel[ch].trackup, 4);
					if(fields & 4) {
						cmd = readchunk(&channel[ch].trackup, 4);
						param = readchunk(&channel[ch].trackup, 8);
					}
					if(note) {
						channel[ch].tnote = note + channel[ch].transp;
						if(!instr) instr = channel[ch].lastinstr;
					}
					if(instr) {
						channel[ch].lastinstr = instr;
						channel[ch].inum = instr;
						channel[ch].iptr = 0;
						channel[ch].iwait = 0;
						channel[ch].bend = 0;
						channel[ch].bendd = 0;
						channel[ch].volumed = 0;
						channel[ch].dutyd = 0;
						channel[ch].vdepth = 0;
					}
					if(cmd) runcmd(ch, cmd, param);
				}
			}

			trackpos++;
			trackpos &= 31;
		}
	}

	for(ch = 0; ch < 4; ch++) {
		s16 vol;
		u16 duty;
		u16 slur;

		while(channel[ch].inum && !channel[ch].iwait) {
			u8 il[2];

			readinstr(channel[ch].inum, channel[ch].iptr, il);
			channel[ch].iptr++;

			runcmd(ch, il[0], il[1]);
		}
		if(channel[ch].iwait) channel[ch].iwait--;

		if(channel[ch].inertia) {
			s16 diff;

			slur = channel[ch].slur;
			diff = freqtable[channel[ch].inote] - slur;
			//diff >>= channel[ch].inertia;
			if(diff > 0) {
				if(diff > channel[ch].inertia) diff = channel[ch].inertia;
			} else if(diff < 0) {
				if(diff < -channel[ch].inertia) diff = -channel[ch].inertia;
			}
			slur += diff;
			channel[ch].slur = slur;
		} else {
			slur = freqtable[channel[ch].inote];
		}
		osc[ch].freq =
			slur +
			channel[ch].bend +
			((channel[ch].vdepth * sinetable[channel[ch].vpos & 63]) >> 2);
		channel[ch].bend += channel[ch].bendd;
		vol = osc[ch].volume + channel[ch].volumed;
		if(vol < 0) vol = 0;
		if(vol > 255) vol = 255;
		osc[ch].volume = vol;

		duty = osc[ch].duty + channel[ch].dutyd;
		if(duty > 0xe000) duty = 0x2000;
		if(duty < 0x2000) duty = 0xe000;
		osc[ch].duty = duty;

		channel[ch].vpos += channel[ch].vrate;
	}

	if (!playsong) ply_init();
}
