#! /usr/bin/python
from itertools import groupby
import sys, struct, os
from sprite_encode2 import add_record
from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

# 8 bits format :  nb skip:4, nb blit: 3, eol:1

DEBUG = False
TRANSP=None
MAXSKIP = 7
MAXBLIT = 15

def reduce(c) :
    'R8G8B8A8 to R3G2B2L1'
    r,g,b =[(x*7+127)/255 for x in c[:3]] # convert to three bits
    a=c[3]
    return r<<5 | (g&~1)<<2 | (b&~1) | (g&1) if a>128 else TRANSP

# wrong : pack to bytes !
def p4_encode(data, palette) :
    linedata=[]
    for i in range(0,len(data),4) :
        w=0
        for j in range(4) :
            w |= (palette.index(data[i+j]) if (len(data)>i+j) else 0)<<(j*4)
        linedata.append(w)
    return struct.pack('<%dH'%len(linedata),*linedata)

def image_encode(src,f,frame_height, mode) :
    # Get the alpha band
    alpha = src.split()[-1]
    r,g,b= src.convert('RGB').convert('P', palette=Image.ADAPTIVE, colors=16).convert('RGB').split()
    src=Image.merge("RGBA",(r,g,b,alpha))

    data = [reduce(c) for c in src.getdata()] # keep image in RAM as bytes / None
    w,h=src.size
    palette=sorted([x for x in set(data) if x!=None])

    print '//',len(palette),'colors '

    s_blits = [] # stringified blits for all image

    start_file = f.tell()
    line16=[] # offsets from start as u16 index on  words

    for y in range(h) :
        if y%16==0 :
            ofs = sum(len(x) for x in s_blits)
            line16.append(ofs)

        skipped=0
        blits=[]

        line=data[y*w:(y+1)*w] # byte/none data
        singles=[]
        for c,g in groupby(line, lambda x:x!=TRANSP) :
            t = tuple(g)
            if not c :
                skipped = len(t)
                # if skip too big, split !
                while (skipped>MAXSKIP) :
                    blits.append([MAXSKIP,(),False])
                    skipped -= MAXSKIP
            else :
                # idem
                while t :
                    blits.append([skipped,t[:MAXBLIT],False])
                    skipped=0
                    t=t[MAXBLIT:]

        # enleve derniers blits si vides
        while(blits and blits[-1][1])==() :
            del blits[-1]

        # set EOL
        if blits :
            blits[-1][2]=True
        else :
            blits.append([0,[],True])


        # now encode line : (header + blit) x n
        for skip, blit, eol in blits :
            header=(skip<<4) | (len(blit) << 1) | (1 if eol else 0)
            s = struct.pack('B', header)
            if mode=='p4' :
                s+= p4_encode(blit,palette=palette)
            elif mode=='u8': # keep native
                s+= struct.pack('%dB'%len(blit),*blit)
            else :
                raise ValueError,"bad mode"
            s_blits.append(s)

    data = ''.join(s_blits)
    data+= '\0'*((-len(data))%4)

    # -- save to file

    # save header
    add_record(f,'header',struct.pack("<2I",w,frame_height))

    # save palette
    if mode in ('p4',) :
        add_record(f,'palette',struct.pack("<%dB"%len(palette),*palette))

    # write data
    add_record(f,mode,''.join(s_blits))

    # line16 record
    add_record(f,'line16',struct.pack("%dH"%len(line16),*line16))

    # finish file
    add_record(f,'end','')

    size=f.tell()
    print '// %d bytes (%d/1M), reduction by %.1f'%(size,1024*1024/size,2*float(w)*h/size)


