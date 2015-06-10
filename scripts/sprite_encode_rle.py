#! /usr/bin/python
from itertools import groupby 
from collections import Counter
import sys, struct, os

from sprite_encode2 import add_record, reduce, rgba, modes

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

# data format as list of u32 :  nb:13,2:RFU,1 EOL ; u16 color 

DEBUG = False


def image_encode(src,f,frame_height, quality) : 
    def err(i) : 
        "error function"
        f = 1/(1.+quality*254)
        t=rgba(i)
        c2=(int(int(t[0]*f)/f),int(int(t[1]*f)/f),int(int(t[2]*f)/f),t[3])
        return reduce(c2)

    data = [reduce(c) for c in src.getdata()] # keep image in RAM as RGBA tuples. 
    w,h=src.size

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
        for c,g in groupby(line, key=err) : 
            t = tuple(g)
            n = len(t)
            # take the most frequent reral color
            cnt = Counter(t)
            col = cnt.most_common(1)[0][0]

            blits.append([n,col,False])

        # set EOL 
        if blits : 
            blits[-1][2]=True
        else : 
            blits.append([0,[],True])


        # now encode line : (header + blit) x n
        for num, color, eol in blits :          
            header=(num<<3) | (1 if eol else 0)
            s = struct.pack('<HH', header, color)
            s_blits.append(s)

    # save header
    add_record(f,'header',struct.pack("<2I",w,frame_height)) # 1 frame for now

    # write data
    add_record(f,'rle',''.join(s_blits))

    # line16 record
    add_record(f,'line16',struct.pack("%dH"%len(line16),*line16))

    # finish file
    add_record(f,'end','')

    size=f.tell()

    print '// %d bytes (%d/1M), reduction by %.1f'%(size,1024*1024/size,2*float(w)*h/size)

def image_decode(f) : 

    record=None;palette=None
    while record != modes['end'] :
        record,size=struct.unpack("<2I",f.read(8))
        raw_data = f.read(size)
        f.read(-size%4) # align

        if record==modes['header'] : 
            w,h=struct.unpack("<2I",raw_data)
            print "(header) w: %d, h:%d "%(w,h)

        elif record==modes['end'] : 
            pass
        elif record==modes['line16'] : 
            pass


        elif record == modes['rle'] : 
            print '(data)',len(raw_data),'bytes'
            # read all blits / lines
            y=0; src=0
            tuples=[]
            line=[]
            while y<h : 
                header, color=struct.unpack('<HH',raw_data[src:src+4])
                src +=4

                nb  = (header>>3) # pixels
                eol = header&1

                if DEBUG : 
                    print 'header(nb=%d,color=%d,eol=%d)'%(nb,color,eol),
                    if (eol) : print

                for i in range(nb) : line.append(rgba(color))

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
    parser.add_argument("-q","--quality", help="Quality coef, 0.0 = perfect, 1.0 = zero", type=float, default=0.)
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
    image_encode(src,f,Height,args.quality)

    f.seek(0)
    image_decode(f).save(args.file_out+'.png') #note that only the first is written (but all are decoded)




