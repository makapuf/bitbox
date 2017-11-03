#!/usr/bin/python2
# extract a common palette from N files
# save it to palette.png + encode it to stdout

import sys
from PIL import Image

DEBUG = True

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

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('file_in', nargs='+', help="Input files to extract palette from")
    parser.add_argument("--file_out",help="Output file name.", default='palette.png')
    parser.add_argument('--colors',type=int, help="number of colors in palette",default=255)
    args = parser.parse_args()

    srcs = [Image.open(filein).convert('RGB') for filein in args.file_in] # removing alpha here
    src  = stack_image(srcs)
    
    # set to 256c image
    src_pal=src.quantize(colors=args.colors,method=2)
    if DEBUG: src_pal.save('_debug.png') # after quantization
    
    # save small palette image 
    src_pal=src_pal.crop((0,0,16,(args.colors+15)/16))
    src_pal.putdata(range(args.colors))
    src_pal.save(args.file_out)

    # export to palette.c
    px = src_pal.convert('RGB').load()
    print "#include <stdint.h>"
    print "const uint16_t vga_palette[%d]= {"%args.colors,

    for i in range(args.colors) : 
        if i%16==0 : sys.stdout.write('\n    ')
        c = px[i%16,i/16]
        sys.stdout.write('0x%04x,'%((c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3))

    print '\n};'
