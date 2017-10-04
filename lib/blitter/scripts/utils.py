# utils.py : library for graphical manipulations for misc bitbox utilities

import os
import sys 
import array
import struct

from itertools import chain, groupby
from collections import Counter

from PIL import Image

ALPHA_T=127 # alpha cutoff

TRANSP8=239
TRANSP16=0 
DEBUG=False

DATA_u16 = 0
DATA_u8  = 1
DATA_cpl = 2

def abspath(ref, path) : 
    "transform relative path from ref file to absolute"
    return os.path.join(os.path.dirname(ref),path)

# --- Graphical Utilities 

def quantize_alpha(im,palette) : 
    "take an RGBA image, apply a 8bpp palette and provide an image with 255 colors + 255 color as transparent"
    im.load()
    palette.load()
    
    alpha = im.split()[-1]
    mask  = Image.eval(alpha, lambda a: 255 if a <=ALPHA_T else 0)
    
    #im2   = im.convert('RGB').quantize('P',dither=0,palette=palette.palette) 
    # force quantization with a given palette, but without dithering !
    assert palette.mode=='P'
    im = im._new(im.im.convert("P", 0, palette.im))

    im.paste(255, mask) # Paste the color of index 255 and use alpha as a mask
    im.info['transparency']=255
    return im

def rgba2u16(r,g,b,a) :
    'R8G8B8 to A1R5G5B5'
    return (1<<15 | (r>>3)<<10 | (g>>3)<<5 |b>>3) if a>128 else 0

def u162rgba(x) : 
    'A1R5G5B5 to r8,g8,b8,a8'
    return (((x>>10)&0x1f)<<3,((x>>5)&0x1f)<<3,(x&0x1f)<<3,255) if (x>>15) else (0,0,0,0)

def gen_micro_pal() : 
    # generates a micro palette Image
    pal = []
    for c in range(255) : 
        r = c >> 5
        g = (c & 0b00011000) >>2  | (c&1)
        b = c & 7
        pal += [r<<5|r<<2|r>>1, g<<5|g<<2|g>>1, b<<5|b<<2|b>>1]
    img = Image.new('P',(16,16))
    img.putpalette(pal)
    if DEBUG and False : 
        img.putdata(range(256))
        img.save('_micro.png')
    return img

def cut_image(img,dw,dh) : 
    "cut an image made of tiles into N images. provide tile size"
    nx=img.size[0]/dw
    ny=img.size[1]/dh

    srcs = []
    for j in range(ny) :
        for i in range(nx) :
            srcs.append(img.crop((i*dw,j*dh,i*dw+dw,j*dh+dh)))
    return srcs

def stack_images_vertically(srcs) : 
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

def write_couple_image(file, size, data, palette) : 
    "write a couple-paletted image to disk"
    print >>sys.stderr,'writing',file
    img= Image.new('RGBA',size)
    d = []
    for c in data : 
        cpl = palette[c]
        d.append(cpl[:4])
        d.append(cpl[4:])
    img.putdata(d)
    img.save(file)


# reduce first : ecrase 3 dernier bits (faire mieux dehors) ? 

def dimcube(c) :
    "size of a color cube along each dimension"
    if not c : return [0,0,0,0,0,0]
    return [max(x[dim] for x in c)-min(x[dim] for x in c) for dim in [0,1,2,4,5,6]] # excludes alpha (zero-len)

def sizebin(bin) : 
    return sum(bin.values())*max(dimcube(bin))

def quantize_couples(cpls,nb):

    ''' 
    Couples palette encoder : only for non transparent pixels !
    find a palette of 256 *couples* which has the same size as a 16 color picture but
    - can allow for a greater number of colors (always better than plain 16c)
    - is faster to blit since you're blitting pixels 2 by 2
    You can also still do palette effects.
    '''

    # vector quantize couples
    uniq = set(cpls) 
    if len(uniq)<=nb :
        print " * no need to reduce"
        palette = sorted(uniq) # sort ensures transparent is first 
        invpal = dict((c,n) for n,c in enumerate(palette))
        return palette, invpal

    b=Counter(cpls)
    bins=[(b,sizebin(b))] 

    while len(bins)<nb :
        # find id of largest cube. largest = volume * nb of elements
        if len(bins)==0 : break # we have no more bins - all are stuck now

        cid = max(range(len(bins)),key=lambda i:bins[i][1])
        #cid = max(range(len(bins)),key=lambda i:sum(bins[i].values())) # from time to time ?

        bin,binsz = bins.pop(cid)

        # choose the largest axis to cut (ie with the most standard deviation) 
        # i.e. compute standard deviation for each axis for this bin, find the max
        avgs= []
        maxstdv=0
        for axis in range(8) : # could avoid alpha
            avg =  sum(float(c[axis]*nb) for c,nb in bin.items()) / sum( bin.values() ) # weighted average
            avgs.append(avg)
            stdv = sum((c[axis]-avg)*(c[axis]-avg)*nb for c,nb in bin.items()) / float(sum(bin.values()))  # variance of each component
            if stdv>maxstdv : 
                chosen_axis=axis
                maxstdv=stdv

        # Cut : place back two new, smaller, bins as a partition of two elements.
        # median or average ?
        if True : # average looks often better than median ?
            b1 = {c:n for c,n in bin.items() if c[chosen_axis] >  avgs[chosen_axis] }
            b2 = {c:n for c,n in bin.items() if c[chosen_axis] <= avgs[chosen_axis] }
        else : 
            # order bin elements along chosen axis
            binelts = sorted(bin.items(), key=lambda c: c[0][chosen_axis])
            # find middle index for median
            l = len(binelts)/2
            # cut along median
            b1 = {k:v for k,v in binelts[:l]}
            b2 = {k:v for k,v in binelts[l:]}
        if b1 : bins.append((b1,sizebin(b1)))
        if b2 : bins.append((b2,sizebin(b2)))

    # now find a representant and build a palette - and inverse palette
    palette=[] 
    invpal = {}

    for bin,_ in bins :
        # find best representant of bin : centroid ? most frequent ?
        #acl=tuple(int(round(avg([c[i] for c in cl])/32.)*31) for i in range(8)) # why 32 ? 5 bits RGB 
        acl=tuple(int(sum(c[axis]*nb for c,nb in bin.items()) / sum( bin.values() ) )&~7 for axis in range(8))
        if any( x>255 for x in acl) : print acl, bin

        # cut alpha
        r1,g1,b1,a1,r2,g2,b2,a2 = acl
        acl = (r1,g1,b1, 255 if a1>ALPHA_T else 0, r2,g2,b2, 255 if a2>ALPHA_T else 0)

        if acl not in palette : # avoid doubles
            palette.append(acl)

        # make an invert mapping (asserts not too big !) from mean
        for c in bin.keys() :
            invpal[c]=palette.index(acl) # takes first if already in palette
    return palette, invpal
