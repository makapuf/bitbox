/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef stSong___H
#define stSong___H

#include "stdlib.h"
#include "string.h"
#include "types.h"
#include "ins_part_channel.h"
#include "txt.h"

#include "st_version.h"

#include "stlimits.h"

// välj vad som skall kompileras
class Player;


class stSong
{
friend class Player;
friend class InsPly;
public:

	stSong( txt &flat );
	virtual ~stSong();

	status_t	Load( txt &t);
	status_t	InitCheck();
	
	// returnerar huruvida låten loopade senaste cykeln
	bool		Play( float *Buffer, uint32 FrameCount );

	void		SetPos( uint32 PALs );

	uint32		SimpLen(); // len i pals
	uint32		Sps();

	float	*N2F;
	float	*R2F;
	float cmul[CHN];

private:
	void	Init();
	void	FreeSong();
	
//	uint16 st_version;
	uint16 spspal;

	uint8 channelcount;
	uint8 partcount;
	uint8 instrumentcount;	// anger inte antal instrument, ett för mycket
	
	Channel	*chan;
	Part	*parts;
	Ins		*ins;

	uint8 breakpcount;			// antal breakpoints
	BreakPoint *breakpoints;	// breakpoints
	status_t init;	

	bool looped;
	
	// player
	float *stereobuffer;
	void MemMulAdd( float *d, float *s, uint32 count, float l, float r);
	void MemMulMove( float *d, float *s, uint32 count, float l, float r);
	uint32 PALs;
	uint32 smp_left;
	float *bs;
	Player **p;
	
#ifdef TARGET_EDITOR
	// partplay
	Channel pchan;
	ChStep pchanstep;
	uint32 psmp_left;
	float *pbs;
	Player *pp;
#endif

};

#endif
