'very simple encoder for frame data to anim.bin file. expects 256color data'
'by example, first convert with : mogrify -path out  -ordered-dither o8x8,32 +dither -colors 255 ????.png'

import sys, glob, struct
from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with it.

def reduce(c) :
    'R8G8B8A8 to A1R5G5B5' # XXX Dither
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

outfile=open('anim.bin','wb')
for name in sorted(glob.glob(sys.argv[1])) : # for each input file
    print name,
    src = Image.open(name) 

    assert src.mode == 'P','not palettized image ?'
    col= max(src.getdata())

    pal=src.getpalette()[:3*col]+[0,0,0]*(256-col) # always 256 colors palette
    print col, 'original colors',src.size,

    pal15 = [reduce((pal[pos:pos + 3]+[255])) for pos in xrange(0, len(pal), 3)]

    # first dimensions as hw
    print "dimensions : ",src.size
    
    outfile.write(struct.pack('<L',0xfaceb0b0))
    outfile.write(struct.pack('<2H',*src.size)) 
    outfile.write(struct.pack('<256H',*pal15))  # 256 color palette as halfwords
    outfile.write(src.tostring())              # 64000 byte data
    outfile.write(struct.pack('<L',0xfaceb0b0))
