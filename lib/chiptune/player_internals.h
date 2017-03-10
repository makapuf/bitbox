// internals of player 

struct channel {
	uint8_t			tnum;
	int8_t			transp;
	uint8_t			tnote;
	uint8_t			lastinstr;
	uint8_t			inum;
	uint16_t		iptr;
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
	uint16_t		slur;
};

extern const uint16_t chip_freqtable[];
extern const int8_t chip_sinetable[];
