/* Simple soundengine for the BitBox
 * Copyright 2014-2015, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project
 */

#warning This engine is deprecated , Please use chiptune.h now !

struct unpacker {
	uint16_t	nextbyte;
	uint8_t	buffer;
	uint8_t	bits;
};

struct channel {
	struct unpacker		trackup;
	uint8_t			tnum;
	int8_t			transp;
	uint8_t			tnote;
	uint8_t			lastinstr;
	uint8_t			inum;
	uint16_t			iptr;
	uint8_t			iwait;
	uint8_t			inote;
	int8_t			bendd;
	int16_t			bend;
	int8_t			volumed;
	int16_t			dutyd;
	uint8_t			vdepth;
	uint8_t			vrate;
	uint8_t			vpos;
	int16_t			inertia;
	uint16_t			slur;
};

extern const uint16_t freqtable[];
extern const int8_t sinetable[];

extern struct channel channel[4];
extern uint8_t trackwait;

/** Initialize the player to play the given song.
 * - Oscillators are reset to silence
 * - Song position is reset to 0
 * - Tracks and instruments are loaded from the song.
 *
 * Pass NULL to stop playing and not load a new song.
 */
void ply_init(const uint8_t songlen, const unsigned char* song);

/** Update the player status. This must be called once per frame as long as the
 * song is running.
 */
void ply_update(); // Call this once per frame.
void ply_update_noloop(); // Call this once per frame, but no looping will occur

/** Directly run a command on a channel.
 *
 * Can be used to play sound effects.
 */
void runcmd(uint8_t ch, uint8_t cmd, uint8_t param);
