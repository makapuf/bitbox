#!/usr/bin/python
'An utility to convert chip tracker .song files to C structs'

s='''
struct ChipSong {
	uint16_t songlen; // number of steps in the track sequencer
	uint8_t numchannels; // 4 or 8 channels supported by now
	uint8_t tracklength; // number of notes you can fit in each track.  default is 32
	uint8_t *tracklist; // id of tracks. songlen * numchannels
	int8_t *transpose; // number of semitones. songlen * numchannels
	
	uint16_t **instruments; // pointer to an array of pointers to command<<8 | parameter

	uint32_t **tracks; // array of pointer to u32 elements
	};
'''

# this version will always ignore second command / parameter
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('file_in', nargs='+', help="Input files in order as frames.")
args = parser.parse_args()

song=[]
tracks=[]
track_ids = ['00'] # initial value : map index '00' is 0
instrs=[]
tracklength = 32

print "#include <stdint.h>"
print s

def cmdref(x) : 
	return "0dfijlmtvw~+=".index(chr(int(x,16)))+1 if x != '00' else 0


for name in args.file_in : 
	for l in open(name) : 

		t = l.split()
		if not t : continue
		if t[0]=='songline' : 
			assert int(t[1],16)==len(song), 'bad line number : '+l
			song.append(t[2:])
		elif t[0]=='trackline' : 
			if not tracks or tracks[-1][-1][0] != t[1] :
				tracks.append([])
				track_ids.append(t[1])
			tracks[-1].append(t[1:])
		elif t[0]=='instrumentline' : 
			if not instrs or instrs[-1][-1][0] != t[1] :
				instrs.append([])
			assert int(t[2],16) == len(instrs[-1]), "%d %d "%(len(instrs[-1]), int (t[2],16))
			instrs[-1].append(t[1:])
		elif t[0]=='tracklength' : 
			tracklength = int(t[1],16)

	#from pprint import pprint


	def i8(x) : 
		n=int(x,16)
		return n if n<=127 else n-256

	assert len(song) < 2**16, "song is too long, somehow..."
	numchannels = len(song[0])/2
	assert numchannels == 4 or numchannels == 8, "number of channels should be 4 or 8 (got %d)"%numchannels
	assert tracklength > 0 and tracklength <= 256, "tracklength should be > 0 and <= 256 (got %d)"%tracklength
	print "struct ChipSong %s_chipsong = {"%name.rsplit('.',1)[0]
	print "    // Song "
	print "    .songlen=%d,"%len(song)
	print "    .numchannels=%d,"%numchannels
	print "    .tracklength=%d,"%tracklength
	print "    .tracklist=(uint8_t[]) {"
	print "\n".join("        "+"".join('%d,'%track_ids.index(x) for x in tuple(l[0::2])) for l in song) 
	print "    },"
	print "    .transpose=(int8_t[]){"
	print "\n".join("        %d,%d,%d,%d,"%tuple(i8(x) for x in l[1::2]) for l in song)
	print "    },"
	print "\n    // Instruments"
	#print "    .nb_instruments =%d,"%len(instrs)
	print "    .instruments = (uint16_t* []){"
	for ins in instrs :
		print "        (uint16_t[]){",
		for step in ins : 
			print "0x%02x%s,"%(cmdref(step[2]),step[3]) ,
		print "0x0100 }," # add end of instr
	print "    },"
	print "\n    // Tracks"
	#print "    .nb_tracks = %d,"%len(tracks)
	print "    .tracks = (uint32_t *[]) {"
	for n,(tr,trid) in enumerate(zip(tracks, track_ids)) : 
		print "        (uint32_t[]){",
		lastindex = 0
		for track,index, note, instr, c1,p1,c2,p2 in tr : 
			while lastindex < int(index,16) : 
				print "0x00000000,",
				lastindex += 1
			# XXX always ignore second parameter ? 
			if c2!='00' : print "// WARNING: will ignore commands here"
			c1n = cmdref(c1)
			print "0x%s%x%x%s%s,"%(note,c1n,0xd if instr!='00' else 0,p1,instr), # Check if instr changed !
			#print "0x%s%xd%s%s,"%(note,c1n,p1,instr), # Check if instr changed !
			lastindex += 1
		print "}, // Track %d - '%s' / len: %d "%(n+1, trid, lastindex)
		assert lastindex <= tracklength, "too many notes in your track somehow...?"
	print '    }'
	print '};'
