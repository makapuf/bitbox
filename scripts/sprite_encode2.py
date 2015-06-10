#! /usr/bin/python
from itertools import groupby 
import sys, struct, os

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

# 16 bits format :  nb skip1:7,  RLE:1, eol:1, nb blit1:7
# encode simply as palette merge + trous

# TODO : multiframe, several frame spritesheets
# nice commandline interface
# p2 mode ; couple modes !

FORCE_MODE = None # 'u16'
DEBUG = False

modes = {
    'header':0,
    'palette':1,
    'line16':2,
    'palette_couple':3,
    'u16':1001,
    'p4':1002,
    'p8':1003,
    'c8':1004,
    'rle':1005,
    'end':32767,
}


def add_record(f,type,data) : 
    f.write(struct.pack('<2I',modes[type],len(data))) # write real size, but will add up the 3 bytes if unaligned
    f.write(data)
    padding = '\0'*(-f.tell()%4)
    f.write(padding) # padding
    print "record %20s,  %d bytes"%(type,len(data)+8),'padding : %d'%len(padding) if padding else ''
    assert f.tell()%4==0,'unaligned entry' # always align

def reduce(c) :
    'R8G8B8A8 to A1R5G5B5'
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

def rgba(x) : 
    'A1R1G1B1 r8,g8,b8,a8'
    return (((x>>10)&0x1f)<<3,((x>>5)&0x1f)<<3,(x&0x1f)<<3,255) if x else (0,0,0,0)


def u16_encode(data,palette) : 
    return struct.pack('<%dH'%len(data),*data)

def u16_decode(sdata,n,palette) :  # palette not used here
    return  (rgba(x) for x in struct.unpack('<%dH'%(len(sdata)/2),sdata))

def p8_encode(data, palette) : 
    linedata=[]
    for i in range(0,len(data),4) : # 1 word = 4 pixels
        w=0
        for j in range(4) : # last word
            w |= (palette.index(data[i+j]) if (len(data)>i+j) else 0)<<(j*8)
        linedata.append(w)
    try : 
        return struct.pack('<%dI'%len(linedata),*linedata)
    except struct.error,e : 
        print linedata
        raise

def p8_decode(data,n, palette) : 
    linedata=[]
    for w in struct.unpack('<%dI'%(len(data)/4),data) : 
        for j in range(min(n,4)) : # last word
            linedata.append(palette[w&0xff])
            w = w>>8 
        n -= 4
    return linedata

def p4_encode(data, palette) : 
    linedata=[]
    for i in range(0,len(data),4) : # 1 word = 4 pixels (encode as u16)
        w=0
        for j in range(4) : # last word
            w |= (palette.index(data[i+j]) if (len(data)>i+j) else 0)<<(j*4)
        linedata.append(w)
    return struct.pack('<%dH'%len(linedata),*linedata)

def p4_decode(data,n, palette) : 
    linedata=[]
    for w in struct.unpack('<%dH'%(len(data)/2),data) : 
        for j in range(min(n,4)) : # last word
            linedata.append(palette[w&0xf])
            w = w>>4 
        n -= 4
    return linedata


def image_encode(src,f,frame_height,mode) : 
    # reduce colors ?
    if mode=='p4' : 
        # Get the alpha band
        alpha = src.split()[-1]
        r,g,b= src.convert('RGB').convert('P', palette=Image.ADAPTIVE, colors=16).convert('RGB').split()
        src=Image.merge("RGBA",(r,g,b,alpha))
    elif mode=='p8' : 
        # Get the alpha band
        alpha = src.split()[-1]
        r,g,b= src.convert('RGB').convert('P', palette=Image.ADAPTIVE, colors=256).convert('RGB').split()
        src=Image.merge("RGBA",(r,g,b,alpha))
    
    data = [reduce(c) for c in src.getdata()] # keep image in RAM as RGBA tuples. 
    w,h=src.size
    palette=sorted([x for x in set(data) if x])

    if mode==None : 
        mode='p4' if len(palette)<=16 else 'p8' if len(palette)<=256 else 'u16'
    
    print '//',len(palette),'colors ; using mode ',mode

    encoders = {'p4':p4_encode, 'u16':u16_encode, 'p8':p8_encode}

    s_blits = [] # stringified blits for all image
    
    start_file = f.tell()
    line16=[] # offsets from start as u16 index on  words 

    for y in range(h) :
        if y%16==0 : 
            ofs = sum(len(x) for x in s_blits)
            line16.append(ofs/4) # XXX use /4 but need to align 

        skipped=0
        blits=[]

        line=data[y*w:(y+1)*w] # 16 bit data
        singles=[] 
        for c,g in groupby(line, bool) : 
            t = tuple(g)
            if not c : 
                skipped = len(t)                
                # if skip too big, split !
                while (skipped>127) : 
                    blits.append([127,(),False])
                    skipped -= 127
            else :
                # idem 
                while t :
                    blits.append([skipped,t[:127],False])
                    skipped=0
                    t=t[127:]


        # set EOL 
        if blits : 
            blits[-1][2]=True
        else : 
            blits.append([0,[],True])


        # now encode line : (header + blit) x n
        for skip, blit, eol in blits :          
            header=(skip<<9) | (len(blit) << 1) | (1 if eol else 0)
            s = struct.pack('<H', header)
            s+= encoders[mode](blit,palette=palette) 
            # pad it
            s+= '\000'*((-len(s))%4)
            s_blits.append(s)

    # save header
    add_record(f,'header',struct.pack("<2I",w,frame_height)) # 1 frame for now

    # save palette
    if mode in ('p4','p8') : 
        add_record(f,'palette',struct.pack("<%dH"%len(palette),*palette))
            
    # write data
    add_record(f,mode,''.join(s_blits))

    # line16 record
    add_record(f,'line16',struct.pack("%dH"%len(line16),*line16))

    # now write line 16

    # finish file
    add_record(f,'end','')

    size=f.tell()

    print '// %d bytes (%d/1M), reduction by %.1f'%(size,1024*1024/size,2*float(w)*h/size)

def image_decode(f) : 
    method = {
        modes['p8']:p8_decode, 
        modes['p4']:p4_decode, 
        modes['u16']:u16_decode
        }

    record=None;palette=None
    while record != modes['end'] :
        record,size=struct.unpack("<2I",f.read(8))
        raw_data = f.read(size)
        f.read(-size%4) # align

        if record==modes['header'] : 
            w,h=struct.unpack("<2I",raw_data)
            if DEBUG : print "(header) w: %d, h:%d "%(w,h)

        elif record==modes['palette'] : 
            d= struct.unpack('<%dH'%(size/2),raw_data)
            palette=[rgba(x) for x in d]
            if DEBUG : print '(palette)',len(palette),'colors'
            if DEBUG : print d
            if DEBUG : print palette

        elif record==modes['end'] : 
            pass
        elif record==modes['line16'] : 
            pass


        elif record in (modes['p8'],modes['p4'],modes['u16']) : 
            if DEBUG : print '(data)',len(raw_data),'bytes'
            # read all blits / lines
            y=0; src=0
            tuples=[]
            line=[]
            while y<h : 
                header=struct.unpack('<H',raw_data[src:src+2])[0] ; src +=2
                skip= header>>9
                nb  = (header>>1)&0x7f # pixels
                eol = header&1
                if DEBUG : 
                    print 'header(skip=%d,nb=%d,eol=%d)'%(skip,nb,eol),
                    if (eol) : print


                # fill with transpdata
                if (record==modes['p8']) :
                    nbytes = 4*((nb+3)/4)
                elif (record==modes['p4']):
                    nbytes = 2*((nb+3)/4)
                elif (record==modes['u16']):
                    nbytes = nb*2


                unpacked_data = method[record](raw_data[src:src+nbytes],nb,palette)
                src+=nbytes

                for i in range(skip) : line.append((0,0,0,0))
                line+=unpacked_data

                if eol : 
                    for i in range(w-len(line)) : line.append((0,0,0,0))
                    y += 1
                    tuples +=line
                    line=[]
                # finish line by skipping bytes
                src += (-src%4)

        else : 
            print "unknown record type:",record

    img = Image.new("RGBA",(w,h))
    img.putdata(tuples)
    return img

if __name__=='__main__' : 
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("file_out",help="Output file name. Usually .spr")
    parser.add_argument('file_in', nargs='+', help="Input files in order as frames.")
    parser.add_argument("-m","--mode",help="Encoding mode",choices=('u16','p4','p8'))
    args = parser.parse_args()

    print '\n***',args.file_in,'to',args.file_out,  args

    # prepare input as vertically stacked images
    srcs = [Image.open(filein).convert('RGBA') for filein in sorted(args.file_in)]

    Width,Height = srcs[0].size
    if not all(im.size == (Width,Height) for im in srcs) : 
        print "All images must be the same size !"
        for nm,im in zip(args.file_in,srcs) : 
            print nm,im.size
        sys.exit(0)

    src = Image.new("RGBA", (Width,Height*len(srcs)))
    for i,im in enumerate(srcs) : 
        src.paste(im,(0,i*Height))

    # output 
    f = open(args.file_out,'wb+')
    image_encode(src,f,Height,args.mode)

    f.seek(0)
    image_decode(f).save(args.file_out+'.png') #note that only the first is written (but all are decoded)




