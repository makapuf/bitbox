#! /usr/bin/python2
from itertools import groupby
import os
import sys
import struct
import array

from PIL import Image # using the PIL/PILlow library

'''File FORMAT : 

Pixels are always taken with a 8bit pallette. If specified, it will be used. 
If not, the default micro bitbox palette will be used.

MAGIC word = 1808B17B
width, height, nb_frames, _RFU_ : u16
line16 : height/16 hwords (16) with absolute offset from absolute start of file to start of line

blit : size:6 type:2 ie size<<2 | type
   if type=00 : skip size pixels
   if type=01 : literal, N bytes 
   if type=10 : back reference, 2 bytes little endian : back reference
   if type=11 : fill with color next byte

OU 
    size:5,type:2,eol:1
'''

"""
TODO

file as a parameter with 
use skip16 if image has identical frames.
skip16->frameptr ?
"""

DEBUG = False
MAGIC = 0x1808B17B
MIN_MATCH = 8
MIN_FILL = 4

def match(haystack, needle) : 
    min_match=MIN_MATCH
    max_match=min(len(needle),63)
    for sz in range(max_match,min_match-1,-1) : 
        pos=haystack.find(needle[:sz])
        if pos>=0 : return pos,sz
    return -1,0

def repeats(s) : 
    for i in range(1,len(s)) : 
        if s[i]!=s[0] : return i
    return len(s)

assert repeats('aaaabchdb')==4
assert repeats('abchdb')==1
assert repeats('aaaaaaaa')==8


def image_encode(src) :
    w,h=src.size

    data = tuple(src.getdata())
    encoded=""   # encoded data so far
    line16  = [] # offsets from start as u16 index 

    nonempty = 0
    
    for y in range(h) :
        if y%16==0 :
            line16.append(len(encoded)) # XXX use /4 but need to align ? reuse past data if 16-height is the same ? use 8-h ?

        line=data[y*w:(y+1)*w] # encode line by line
        for is_transp,g in groupby(line, lambda c:c==src.info['transparency']) : # but into blits  / transp
            if is_transp :
                skipped = sum(1 for x in g)
                # if skip too big, split !
                while (skipped) :
                    n=min(skipped, 63)
                    encoded += chr(n)
                    skipped -= n
                    if DEBUG : print 'skip',n
            else :
                # raw string to encode.
                s = array.array('B',g).tostring()
                nonempty+=len(s) 
                buf=''
                while s :
                    pos,n  = match(encoded,s)
                    repeat = repeats(s)
                    if repeat>=MIN_FILL : 
                        repeat = min(repeat,63)
                        if buf : 
                            encoded += chr(len(buf)<<2|1)+buf # send raw data
                            if DEBUG : print 'literal',repr(buf)
                            buf=''
                        encoded += chr(repeat<<2|3)+s[0]
                        if DEBUG : print 'repeat',repeat,repr(s[0])
                        s = s[repeat:]
                        
                    elif pos>=0 : # back reference
                        if buf : 
                            encoded += chr(len(buf)<<2|1)+buf # send raw data
                            if DEBUG : print 'literal',repr(buf)
                            buf=''
                        encoded += struct.pack('<BH',n<<2|2, pos)
                        s = s[n:]
                        if DEBUG : print 'backref',n,pos
                    else : # append one to last blit or create one
                        buf += s[0]
                        s=s[1:]
                if buf :   
                    encoded += chr(len(buf)<<2|1)+buf
                    if DEBUG : print 'literal',repr(buf)
                    buf=''
        if DEBUG : print

    print '// %d bytes (%d/1M), reduction by %.1f'%(len(encoded),1024*1024/len(encoded),2*float(w)*h/len(encoded))
    print '// non empty/8bpp : %d'%nonempty
    return line16, encoded


def stack_images_vertically(srcs) : 
    # prepare input as vertically stacked images and return a big string 
    Width,Height = srcs[0].size
    if not all(im.size == (Width,Height) for im in srcs) :
        print "All images must be the same height !"
        for im in srcs : 
            print getattr(im,'filename','??') ,im.size

        sys.exit(1)

    src = Image.new("RGBA", (Width,Height*len(srcs)))
    for i,im in enumerate(srcs) :
        src.paste(im,(0,i*Height))
    return src

def cut_image(img,dw,dh) : 
    nx=img.size[0]/dw
    ny=img.size[1]/dh

    srcs = []
    for j in range(gh) :
        for i in range(gw) :
            srcs.append(img.crop((i*dw,j*dh,i*dw+dw,j*dh+dh)))


def quantize_alpha(im,palette) : 
    im.load()
    palette.load()
    
    alpha = im.split()[-1]
    mask  = Image.eval(alpha, lambda a: 255 if a <=128 else 0)
    
    #im2   = im.convert('RGB').quantize('P',dither=0,palette=palette.palette) 
    # force quantization with a given palette, but without dithering !
    assert palette.mode=='P'
    im = im._new(im.im.convert("P", 0, palette.im))

    im.paste(255, mask) # Paste the color of index 255 and use alpha as a mask
    im.info['transparency']=255
    return im

def write_frames(srcs, f) : 
    src = stack_images_vertically(srcs)

    palette=Image.open('palette.png')
    img=quantize_alpha(src,palette)
    if DEBUG : img.save('_debug.png')
    
    line16,encoded = image_encode(img)

    # output
    
    f.write(struct.pack('IHHHH',MAGIC, img.size[0], img.size[1]/len(srcs), len(srcs),0))
    f.write(struct.pack('%dH'%(img.size[1]/16),*line16))
    f.write(encoded)


if __name__=='__main__' :
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("file_out",help="Output file name. Usually .spr")
    parser.add_argument('file_in', nargs='+', help="Input files in order as frames. Must be encoded with 256c palette.")
    parser.add_argument('--size', help="WxH size of a frame in pixels.")
    
    args = parser.parse_args()

    if args.geometry : 
        gw,gh = (int(x) for x in args.geometry.split("x"))
    else : 
        gw,gh = Image.open(args.file_in[0]).size

    print '\n***',args.file_in,'to',args.file_out,'frame size',gw,'x',gh

    srcs = []
    for file_in in sorted(args.file_in) : 
        img = Image.open(file_in).convert('RGBA')
        srcs += cut_image(img,gw,gh)

    write_frames(srcs, open(args.file_out,'wb+'))
