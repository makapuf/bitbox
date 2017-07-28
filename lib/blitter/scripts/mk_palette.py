#!/usr/bin/python2
# extract a common palette from N files
# save it to palette.png / stdout

import sys
from PIL import Image

DEBUG = False

def stack_image(images) : 
    "stack vertically, expanding as needed"
    w = max(i.size[0] for i in images)
    h = sum(i.size[1] for i in images)
    newimg = Image.new('RGB',(w,h))
    y=0
    for im in images : 
        newimg.paste(im,(0,y))
        y += im.size[1]
    return newimg

if __name__=='__main__' :
    srcs = [Image.open(filein).convert('RGB') for filein in sys.argv[1:]] # removing alpha here
    src  = stack_image(srcs)
    
    # set to 256c image
    #src_pal=src.convert('P',colors=256,palette=Image.ADAPTIVE)
    src_pal=src.quantize(colors=255,method=0)
    if DEBUG: src_pal.save('_debug.png') # after quantization
    
    # save small palette image 
    src_pal=src_pal.crop((0,0,16,16))

    src_pal.putdata(range(256))
    src_pal.save('palette.png')

    # export to palette.c
    px = src_pal.convert('RGB').load()
    print "#include <stdint.h>"
    print "const uint16_t vga_palette[256]= {"
    for i in range(16) : 
        s = ''.join('0x%04x,'%((px[j,i][0]>>3)<<10 | (px[j,i][1]>>3)<<5 | px[j,i][2]>>3)  for j in range(16))
        print '    '+s
    print '};'
