#!/usr/bin/python
import struct, sys
from cStringIO import StringIO


DEFAULT_MIDI_HEADER_SIZE = 14
EVENT_TYPES = { # value : type, p1,p2
    0x8 : ('Note Off', "note","velocity",'midi_note_off'),
    0x9 : ('Note On','note','velocity','midi_note_on'),
    0xA : ('Note Aftertouch','note','aftertouch value','midi_note_aftertouch'),
    0xB : ('Controller','controller','controller value','midi_controller'),
    0xC : ('Program Change','program',None,'midi_program'),
    0xD : ('Channel Aftertouch','aftertouch value',None,'midi_chan_aftertouch'),
    0xE : ('Pitch Bend','pitch value (LSB)','pitch value (MSB)','midi_pitch_bend'),
}
META_TYPES = {
     1 : 'Text',
     2 : 'Copyright',
     3 : 'Track Name',
     4 : 'Instrument Name',
    47 : 'End of Track',
    81 : 'Set Tempo',
    88 : 'Time Signature'
}
NOTES = 'C,C#,D,D#,E,F,F#,G,G#,A,A#,B'.split(',')

def read_varlen(data):
    value = 0
    while True :
        x = read_fmt(data,'B')[0]
        value = (value<<7) + (x&0x7f) # shift and add
        # is the hi-bit clear? end
        if not (x & 0x80): break
    return value

def read_fmt(file,format) :
    return struct.unpack(format,file.read(struct.calcsize(format)))

def read_midi(midifile):
    format,ntracks,resolution = parse_file_header(midifile)
    assert format==1,'only midi type 1 are readable'
    assert 0<resolution<1000,'strange resolution (frames related ?)'
    assert ntracks>1,"at least 2 tracks"

    tempo = 120

    print "// %d tracks, %d ticks/beat (PPQ)"%(ntracks,resolution)
    tot_evts = 0
    for i in range(ntracks) :
        events, set_tempo, track_name=parse_track(midifile)
        if set_tempo : tempo = set_tempo
        if not events : continue # skip empty track - First Type 1 midi often
        print 'Track track_%s={'%track_name
        for tick, evttype, p1, p2, comment in events : 
            print '    {.tick=%4d, .type=%s, .p1=0x%02x, .p2=0x%02x}, // %s'%(tick,evttype, p1, p2, comment)
        tot_evts += len(events)
        print '}'

    print '// Total song : ',tot_evts,'events',tot_evts*8//1024,'kb'
    return format,ntracks,resolution,tot_evts

def read_midi_simple(midifile):
    BITBOX_PPQ = 24 
    format,ntracks,resolution = parse_file_header(midifile)
    assert format==1,'only midi type 1 are readable'
    assert 0<resolution<1000,'strange resolution (frames related ?)'
    assert ntracks>1,"at least 2 tracks"

    tempo = 120

    print "// %d tracks, %d ticks/beat (PPQ)"%(ntracks,resolution)

    tot_evts = 0
    for i in range(ntracks) :
        events, set_tempo, track_name=parse_track(midifile)
        # escapes track name, only keep alnums and replace rest with _
        track_name=''.join(c if c.isalnum() else '_' for c in track_name)

        if set_tempo : tempo = set_tempo
        if not events : continue # skip empty track - First Type 1 midi often
        print 'struct NoteEvent track_%s[] = {'%track_name.lower()
        for tick, evttype, p1, p2, comment in events : 
            frame = round(float(tick)*BITBOX_PPQ/resolution)

            if evttype=='midi_note_on' : 
                print '    {.tick=%4d, .note=0x%02x, .vel=0x%02x}, // %s'%(frame, p1, p2, comment)
            elif evttype=='midi_note_off':
                print '    {.tick=%4d, .note=0x%02x, .vel=0}, // note off '%(frame, p1)
            else : 
                pass # ignore

        tot_evts += len(events)
        print '};'
        
    print 'const int track_%s_len = %d; // %d kb'%(track_name.lower(),tot_evts,tot_evts*4//1024)
    return format,ntracks,resolution,tot_evts


def parse_file_header(midifile):
    # First four bytes are MIDI header
    assert midifile.read(4) == 'MThd',"Bad header in MIDI file."

    # next four bytes are header size
    # next two bytes specify the format version
    # next two bytes specify the number of tracks
    # next two bytes specify the resolution/PPQ/Parts Per Quarter
    # (in other words, how many ticks per quater note)
    hdrsz,format,ntracks,resolution = read_fmt(midifile,">LHHH")
    #tracks = [Track() for x in range(data[2])]

    # XXX: the assumption is that any remaining bytes
    # in the header are padding
    if hdrsz > DEFAULT_MIDI_HEADER_SIZE:
        midifile.read(hdrsz - DEFAULT_MIDI_HEADER_SIZE)
    return format,ntracks,resolution

def parse_track(midifile):
    tempo = None # default value
    track_name = None

    RunningStatus = None
    events = []

    assert midifile.read(4)=='MTrk', "Bad track header in MIDI file on byte %d."%(midifile.tell()-4)
    # next four bytes are track size
    trksz = read_fmt(midifile,">L")[0]
    #print 'track size:%d'%trksz
    trackdata=midifile.read(trksz)
    trackfile=StringIO(trackdata)

    # now parse events from that track
    while True :
        # debug
        #n=trackfile.tell()
        #print '>',' '.join(["%02x"%ord(x) for x in trackfile.read(15)])
        #trackfile.seek(n)

        tick   = read_varlen(trackfile)
        status = read_fmt(trackfile,'B')[0]


        if status == 0xff : # meta event
            meta_type = read_fmt(trackfile,'B')[0]
            data_len = read_varlen(trackfile)
            meta_data = trackfile.read(data_len)
            if meta_type == 47 : # end of track
                #print '// (%d) - <END OF TRACK>'%tick
                break # exit while loop
            elif meta_type == 81 : 
                # 3 bytes big endian
                tempo_MPQN = ((ord(meta_data[0]))<<16)+((ord(meta_data[1]))<<8)+ord(meta_data[2])
                tempo_BPM = 60000000 / tempo_MPQN
                print '// Set Tempo : %d bpm (%d mpqn)'%(tempo_BPM, tempo_MPQN)
                tempo = tempo_BPM
            elif meta_type == 88 : # time signature
                num = ord(meta_data[0])
                den = 2**ord(meta_data[1])
                # metro : whatever ord(meta_data[2])+ord(meta_data[3])/32.
                print '// Time signature : %d/%d'%(num,den)
            elif meta_type == 3: # track name
                # XXX check first track ?
                track_name = meta_data
                print '// Track name :',track_name
            else : 
                print '// (meta) %s :'%META_TYPES.get(meta_type,'??? (%d)'%meta_type),repr(meta_data)

        elif status in (240,247) :
            sysex_len = read_varlen(trackfile)
            sysex_data = trackfile.read(sysex_len)
            print "sysex : ",sysex_data

        else : # event (with running status ?)

            if status&0x80 :
                p1 = read_fmt(trackfile,'B')[0]
                RunningStatus=status
            else : # use running status
                status = RunningStatus
                p1 = status
                #print '*',

            evt_type = status >> 4
            channel = status & 0xf
            s_type,s_p1,s_p2,evt_enum = EVENT_TYPES[evt_type]
            if s_p2 :
                p2 = read_fmt(trackfile,'B')[0]
            else :
                p2=0
            comment = 'ch %d : %s=%s'%(channel,s_p1,p1)
            # ignore channel
            if evt_type==0x9 : # note_on
                comment += ' '+NOTES[p1%12]+str(1+p1/12)
            if s_p2 :
                comment += ' %s=%s'%(s_p2,p2)
            events.append((tick,evt_enum,int(p1),int(p2),comment))

    return events, tempo,track_name


if __name__=='__main__' :
    filename = sys.argv[1] 
    file = open(filename,'rb')
    print '#include "sampler.h"'
    print "// file ", filename
    print '// format %d,ntracks %d,resolution %d, %d events'%read_midi_simple(file)