/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h> // for PRIu32

#include "stSong.h"
#include "txt.h"
#include "player.h"

void *CALLOC(int s1, int s2);
void *CALLOC(int s1, int s2)
{
	char *tmp = (char*)malloc(s1 * s2);
	for (int c = 0; c< (s1*s2) ; c++) tmp[c]=0;
	return (void*)tmp;
}

void stSong::Init()
{
	//////////////////////////////////////////////////////// calc-multiplyers
	const float floor = (float)0.1;
	{
		cmul[0] = 0;
		for (uint32 c = 1 ; c < CHN ; c++){
			cmul[c]=((1-floor)/(float)c)+floor;
			//printf("[%" PRIu32 "] == %f\n",c,cmul[c]);
		}
	}

	//////////////////////////////////////////////////////// FREQ-TABLES
	const double MUL = 1.0594630943593;

	N2F = (float *) malloc(22*12*sizeof(float));
	R2F = (float *) malloc(22*12*sizeof(float));

	int count = 0;
	double octbase=1.02197486445547712033;

	{
		for (int oc=0;oc<22;oc++){
			double base=octbase;
			for (int c=0;c<12;c++){
				N2F[count++] = (float)base;
				base *= MUL;
			}
			octbase *= 2;
		}
	}

	count = 0;
	octbase = 1;
	stereobuffer = 0;
	 	
	{
		for (int oc=0;oc<22;oc++){
			double base=octbase;
			for (int c=0;c<12;c++){
				R2F[count++] = (float)base;
				base *= MUL;
			}
			octbase *= 2;
		}
	}

	//////////////////////////////////////////////////////// END OF FREQ-TABLES

	channelcount = 0;
	partcount = 0;
	instrumentcount = 0;
	breakpcount = 0;

	looped = false;
	p = 0;
	chan = 0;
	parts = 0;
	ins = 0;
	breakpoints = 0;
	PALs = 0;
	stereobuffer = 0;

#ifdef TARGET_EDITOR
	pp = 0;
#endif

}

/*_EXPORT*/ stSong::stSong(txt &flat)
{
	Init();
	init = JNG_OK;
	
	///////////////////////////////////////	här allokeras elände till sången...
	
	if (JNG_OK != (init = Load(flat))) return;

	stereobuffer = new float[spspal * 2];
	bs = stereobuffer;

	///////////////////////////////////////	partplayer
#ifdef TARGET_EDITOR
		pchan.steps = &pchanstep;
		pchan.len = 1;
		pchan.rloop = 0;
		pchan.lloop = 0;
		pchanstep.part = 0;
		pchanstep.transp = 0;
		pp = new Player(this,&pchan);
		pbs = stereobuffer;
#endif

	SetPos(0);
	return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*_EXPORT*/ status_t stSong::Load(txt &t)
{
#ifdef TARGET_EDITOR
	FreeSong();
#endif
	uint32 c, c1/*, num*/;
	if (JNG_OK!=t.InitCheck()){
		init=JNG_ERROR;
		return init;
	};

	/*num =*/ t.NextVal(4);
#if 0
	switch (num){
		case 'SWTD': 		break;
		case 'SWTT': t.IsBin(false);	break;
		default : return init=JNG_ERROR;
	}
#endif

	/*st_version =*/ t.NextVal(2);

#if 0
	if (st_version > ST_CURRENT_FILE_VERSION){
		return init = JNG_ERROR_VERSION;
	}
		
	if (st_version < 900 )			// special-hack för kompatibilitet med CLOSED_BETA
		spspal = 882;
	else
#endif
		spspal = t.NextVal(2);
		

	if (spspal < 1){
		return init = JNG_ERROR;
	}

	//////////////////////////////////////////////////////// CHANNELS
	channelcount = t.NextVal();		// no. of channels
	if (channelcount < 1){
		return init = JNG_ERROR;
	}

#ifdef TARGET_EDITOR
	chan = (Channel*) CALLOC( CHN , sizeof(Channel));	// allocates more space for editor
#else
	chan = (Channel*) CALLOC( channelcount , sizeof(Channel));
#endif

	for (c=0; c < channelcount; c++){
		chan[c].left = t.NextVal();
		chan[c].right = t.NextVal();
		chan[c].len = t.NextVal(2);

		// previous we did not have loop points
#if 0
		if (st_version < 910){
			chan[c].lloop = 0;
		}else
#endif
		{
			chan[c].lloop = t.NextVal(2);
		}
		
		// previous we did not have right-loop points
#if 0
		if (st_version < 1200)
			chan[c].rloop = chan[c].len-1;
		else
#endif
			chan[c].rloop = t.NextVal(2);
			
		if (1 > chan[c].len){
			init=JNG_ERROR;
			return init;
		}

		if (CHNSTEPS < chan[c].len){
			init=JNG_ERROR;
			return init;
		}

		if (chan[c].rloop >= chan[c].len){
			chan[c].rloop = chan[c].len-1;
		}

		
		//	seq
#ifdef TARGET_EDITOR
		chan[c].steps = (ChStep*) CALLOC( CHNSTEPS , sizeof(ChStep));
#else
		chan[c].steps = (ChStep*) CALLOC( chan[c].len , sizeof(ChStep));
#endif

		{
			for (c1=0; c1 < chan[c].len; c1++){
				chan[c].steps[c1].part = t.NextVal();
				chan[c].steps[c1].transp = t.NextVal();
				chan[c].steps[c1].damp = t.NextVal();
			}
		}
	}
	
	//////////////////////////////////////////////////////// PARTS
	partcount = t.NextVal();
	if (1 > partcount){init=JNG_ERROR;return init;}

#ifdef TARGET_EDITOR
	parts = (Part*) CALLOC( PARTS , sizeof(Part));	// allocates more space for editor
#else
	parts = (Part*) CALLOC( partcount , sizeof(Part));
#endif

	for (c=0; c < partcount; c++){
		parts[c].sps = t.NextVal();
		if (parts[c].sps < 1){
			return init = JNG_ERROR;
		}

		parts[c].len = t.NextVal();
		if (parts[c].len < 1){
			return init = JNG_ERROR;
		}

#ifdef TARGET_EDITOR
		parts[c].steps = (Step*) CALLOC( PARTSTEPS , sizeof(Step));
#else
		parts[c].steps = (Step*) CALLOC( parts[c].len , sizeof(Step));
#endif

		for (c1=0; c1 < parts[c].len; c1++){
			parts[c].steps[c1].ins = t.NextVal();
			parts[c].steps[c1].eff = t.NextVal();
			parts[c].steps[c1].note = t.NextVal();
		}
	}

	// testa om man laddat in nått i kanalerna som refererar till en part som inte finns
	for (c = 0 ; c < channelcount ; c++){	
		for (c1=0; c1 < chan[c].len; c1++){
			if (chan[c].steps[c1].part >= partcount){
				chan[c].steps[c1].part = partcount -1;
			}
		}
	}
		
		
	//////////////////////////////////////////////////////// INSTRUMENTS

	instrumentcount = 1+t.NextVal();
	if (instrumentcount < 2){
		return init = JNG_ERROR;
	}

#ifdef TARGET_EDITOR
	ins = (Ins*) CALLOC( INS , sizeof(Ins));	// allocates more space for editor
#else
	ins = (Ins*) CALLOC( instrumentcount , sizeof(Ins));
#endif

// allocate dummy instrument, is this really needed nowdays?
	ins[0].filterpoints	= 1;
	ins[0].amppoints	= 1;
	ins[0].filter		= (TimeLev*) CALLOC( 1 , sizeof(TimeLev));
	ins[0].amp			= (TimeLev*) CALLOC( 1 , sizeof(TimeLev));

	ins[0].filtermode = 0;
	ins[0].clipmode = 0;
	ins[0].boost = 1;

	ins[0].sps = 30;
	ins[0].res = 0;

	ins[0].vibs = 1;
	ins[0].vibd = 1;

	ins[0].pwms = 1;
	ins[0].pwmd = 1;

	
	ins[0].len = 1;
	ins[0].loop = false;
	
	ins[0].steps = (InsStep*) CALLOC(1 , sizeof(InsStep));
// end of dummy ins allocation
	

	for (c = 1; c < instrumentcount; c++)
		if (JNG_OK !=ins[c].Load(t)){
			return init = JNG_ERROR;
		}

	
	/////////////////// Load BreakPoints
	breakpcount = 0;
	breakpcount = t.NextVal();

#ifdef TARGET_EDITOR
		breakpoints = (BreakPoint*) CALLOC( BREAKPOINTS , sizeof(BreakPoint*));
#else
		breakpoints = (BreakPoint*) CALLOC( breakpcount , sizeof(BreakPoint*));
#endif

	{
		for (int c = 0 ; c < breakpcount ; c++){
			breakpoints[c].PAL = t.NextVal(4);
			breakpoints[c].command = t.NextVal(4);
		}
	}
	
	/////////////////// Load Names
	/*name = strdup*/(t.NextString());
	/*author = strdup*/(t.NextString());

	for (int c = 0 ; c < partcount ; c++){
		/*parts[c].SetName*/(t.NextString());
	}
	for (int c = 1 ; c < instrumentcount ; c++){
		/*ins[c].SetName*/(t.NextString());
	}

	/////////////////// Create Playaz
#ifdef TARGET_EDITOR
	p = (Player **)CALLOC(CHN , sizeof(Player *));
#else
	p = (Player **)CALLOC(channelcount , sizeof(Player *));
#endif
	{
		for (int c = 0; c< channelcount; c++){
			p[c] = new Player(this,chan + c);
		}		
	}

	return InitCheck();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*_EXPORT*/ status_t stSong::InitCheck(){
// check for big instruments
	for (int c = 0 ; c < partcount ; c++){
		for (int c2 = 0 ; c2 < parts[c].len ; c2++){
			if (parts[c].steps[c2].ins >= instrumentcount){
				return JNG_ERROR;
			}
		}
	}

	return init;
}

/*_EXPORT*/ void stSong::FreeSong()
{
#ifdef TARGET_EDITOR
	if (pp) delete pp;
#endif

	int c;
	//////////////////////////////////////////////////////// CHANNELS
	for (c=0; c < channelcount; c++)
	{
		if (p) if (p[c]) delete p[c];
		if (chan[c].steps) free( chan[c].steps );
	}

	if (chan){
		free( chan );
		if (p) free( p );
		chan = 0L;
		channelcount = 0;
	}
	
	//////////////////////////////////////////////////////// PARTS
	for (c=0; c < partcount; c++){
		//if ( parts[c].name ) free( parts[c].name );
		if ( parts[c].steps )free( parts[c].steps );
	}

	if (parts){
		free( parts );
		parts = 0L;
		partcount = 0;
	}
	
	//////////////////////////////////////////////////////// INSTRUMENTS
	for (c = 0; c < instrumentcount; c++) ins[c].Free();
	
	if (ins){
		free( ins );
		ins = 0L;
		instrumentcount = 0;
	}
	
	
	//////////////////////////////////////////////////////// SONG STUFF
	if (breakpoints) free( breakpoints );
	breakpcount = 0;
}

/*_EXPORT*/ stSong::~stSong()
{
	if (0 != stereobuffer) delete stereobuffer;

	free( N2F );
	free( R2F );
	FreeSong();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*_EXPORT*/ bool stSong::Play(	float *buff,
					uint32 FrameCount	)
{
// ----------------------
// SoundPart

	float *bd = buff;
//	float *bs = p[0]->Buffer();

// skall köra allas buffrar
	while (FrameCount > 0){
		uint32 lim;
		if (FrameCount < smp_left){
			lim = FrameCount;
			FrameCount = 0;
			smp_left -= lim;
		}else{
			lim = smp_left;
			smp_left = 0;
			FrameCount -= lim;
		}
		
		lim <<=1; // stereo
		while ( lim ){
			*bd = *bs;
			bd++;
			bs++;
			lim--;
		}
		
		
		float channelmul = cmul[channelcount] / 2;

		// a bit optimized
		if (smp_left < 1){
			//////////// NextBuffer and move the first one found
			int c = 0;
			for (; c < channelcount ; c++){
				if ( p[c]->NextBuffer() ){
					MemMulMove(stereobuffer,p[c]->Buffer(), spspal,
						(float)chan[c].left  * channelmul,
						(float)chan[c].right * channelmul);
					break;
				}
			}			
			c++;

			// after that, add the next buffers
			if (c > channelcount )
				memset( stereobuffer, 0, spspal * sizeof(float) * 2);
			else{
				for (; c < channelcount ; c++){
					if (p[c]->NextBuffer())
						MemMulAdd( stereobuffer, p[c]->Buffer(), spspal,
						(float)chan[c].left  * channelmul,
						(float)chan[c].right * channelmul);
				}
			}

			//////////// PAL looping
			PALs++;
			if (p[0]->Looped()){
				looped = true;
				PALs = 0;
				for ( int cnt = 0 ; cnt < chan[0].lloop ; cnt++){
					PALs += ( parts[chan[0].steps[cnt].part].sps * parts[chan[0].steps[cnt].part].len );
				}
//				printf("PALs == %d\n",PALs);
			}

			smp_left = spspal;
			bs = stereobuffer;
		}
		
	}
	
// ----------------------
// BreakPointPart
	if (looped){
		looped = false;
		return true;
	} else return false;
}


inline void stSong::MemMulMove( float *d, float *s, uint32 count, float l, float r)
{
	while ( count-- ){
		d[0] = *s * l;
		d[1] = *s * r;
		d+=2;
		s++;
	}
}

inline void stSong::MemMulAdd( float *d, float *s, uint32 count, float l, float r)
{
	while ( count-- ){
		d[0] += *s * l;
		d[1] += *s * r;
		d+=2;
		s++;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*_EXPORT*/ void stSong::SetPos(uint32 pals)
{
	smp_left = 0;
	PALs = pals;

	for (int c=0;c<channelcount;c++){
		// set seqcount
		int32	sum = 0;
		int32	seqcount = 0, stepcount = 0, palcount = 0, sps = 0, len = 0;
		
		while (sum <= (int32)PALs){
			len = parts[chan[c].steps[seqcount].part].len;
			sps = parts[chan[c].steps[seqcount].part].sps;
			sum += sps * len;

			if (seqcount > chan[c].rloop) seqcount=chan[c].lloop;
			seqcount++;
		}

		if (seqcount > 0)
			seqcount--;
		else
			seqcount = chan[c].rloop;

		len = parts[chan[c].steps[seqcount].part].len;
		sps = parts[chan[c].steps[seqcount].part].sps;

		sum -= sps * len;

			
		stepcount	= (PALs - sum) / sps;
		palcount	= (PALs - sum) % sps;

		p[c]->JumpPos((uint8)seqcount,(uint8)stepcount,(uint8)palcount);
//		printf("PALs[%d] == Seq[%d] Step[%d] Rest[%d]\n",PALs,seqcount,stepcount,palcount);

		if (c == 0){
		// snygga till PAL
			PALs = 0;
			for ( int cnt = 0 ; cnt < seqcount ; cnt++){
				PALs += ( parts[chan[0].steps[cnt].part].sps * parts[chan[0].steps[cnt].part].len );
			}
			PALs += stepcount * parts[chan[0].steps[seqcount].part].sps;
			PALs += palcount;
		}
	}

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*_EXPORT*/ uint32 stSong::SimpLen()
{
	int max = 0;
	// beräkna sångens längd

	for (int cc = 0 ; cc < channelcount ; cc++){
		int bufc = 0;	// number of buffers
		for (int c = 0 ; c < chan[0].len ; c++){
			Part *tpart = parts + (chan[0].steps[c].part);
			bufc += tpart->sps * tpart->len;			
		}

		if (bufc > max) max = bufc;
	}
	return max;
}

/*_EXPORT*/ uint32 stSong::Sps()
{
	return spspal;
}


