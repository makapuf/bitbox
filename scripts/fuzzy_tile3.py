#!/usr/bin/python
"Encode as 16x16 palette, lossy, using RLE or not for map, 16 colors per tile"

'''TODO : faster encode, better line interface, stats on error
'''


from math import sqrt
from itertools import groupby
import sys
from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

LAMBDA = 5000  # max average deviation per pixel,
BLOCKSIZE = 16
TRANSP = (0,0,0)

# HIGHER quality : find centroids of squares, THEN reduce (...)

def output_tileset() :
    tiles_per_row = 16
    tileset_image = Image.new("RGB",(tiles_per_row*BLOCKSIZE,(len(tileset)+(tiles_per_row-1))/tiles_per_row*BLOCKSIZE))
    for i,t in enumerate(tileset) :
        imgblock.putdata(t)
        tileset_image.paste(imgblock,(i%tiles_per_row*BLOCKSIZE,i/tiles_per_row*BLOCKSIZE))
    tileset_image.save(name+'_tileset.png')


yuvs = {} # memo
def blocktoYUV(block) :
    try :
        return yuvs[block]
    except KeyError :
        x = [(
         0.299 * r + 0.587 * g + 0.114 * b,
        -0.147 * r - 0.289 * g + 0.436 * b,
         0.615 * r - 0.515 * g - 0.100 * b)
        for r,g,b in block ] # Y,U,V
        yuvs[block]=x
        return x


def distance(block1,block2, maxdist) :
    # sum sq difference over each band over each pixel.
    # YUV MODE, Y 3x more important => only Y ?
    yb1 = blocktoYUV(block1)
    yb2 = blocktoYUV(block2)
    #return sum( (c1[0]-c2[0])**2 for c1,c2 in zip(yb1, yb2)) / BLOCKSIZE/ BLOCKSIZE*100. # luminance only
    #return sum(sum((b1-b2)**2) for b1,b2 in zip (c1,c2)) for c1,c2 in zip(block1, block2)) / BLOCKSIZE/ BLOCKSIZE*100. # RGB
    err=0.
    for c1,c2 in zip(yb1, yb2) :
        err += sum(p*((b1-b2)**2) for b1,b2,p in zip (c1,c2,[3,1,1]))/len(block1)
        if err>maxdist : break
    return err

def stats():
    print ' -',len(tileset),'/',len(tilemap), len(tileset)*100 // len(tilemap),'%'

for name in sys.argv[1:] : # for each input file
    print " *** ",name

    tileset = [tuple([TRANSP])*BLOCKSIZE*BLOCKSIZE] # only one empty tile
    tilemap = []

    src = Image.open(name+'.png').convert('RGB') # XXX ALPHA
    dst = src.copy()

    encoded_blocks = []
    print '// blocks  , lambda :%.1f, blocksize: %d '%(LAMBDA, BLOCKSIZE)

    for dim in 0,1 :
        if src.size[dim]%BLOCKSIZE!=0 :
            print 'file %s : %s size not a multiple of %d '%(name,'xy'[dim],BLOCKSIZE)
            continue

    for y in range(src.size[1]//BLOCKSIZE) :
        for x in range(src.size[0]//BLOCKSIZE) :
            imgblock = src.crop((x*BLOCKSIZE,y*BLOCKSIZE,(x+1)*BLOCKSIZE,(y+1)*BLOCKSIZE))
            block = imgblock.getdata()

            # find closest tile and get error
            min_err = LAMBDA ; min_idx = 0
            for idx in range(len(tileset)) :
                err = distance(tileset[idx],block, min_err)
                if err<min_err :
                    min_err = err
                    min_idx = idx
                if err==0.0 : break

            if min_err >= LAMBDA :
                min_idx=len(tileset)
                tileset.append(block)
            else :
                imgblock.putdata(tileset[min_idx])
                dst.paste(imgblock,(x*BLOCKSIZE,y*BLOCKSIZE))
                # mark with 3 small pixels it's a reused tile ?
            tilemap.append(min_idx)

        if y%4==0 :
            stats()

            dst.save(name+'decoded.png')
            output_tileset() # optional ?

    encoded_size = BLOCKSIZE*BLOCKSIZE*len(tileset)
    stats()
    print 'encoded size:',encoded_size, 'reduc : %.1f'%(src.size[0]*src.size[1]*8./encoded_size)


    # faire une option : generate png or use it (to create TMX) - permet rearrangement, prend le plus proche

    # generate TMX
    with open(name+'.tmx','w') as of :
        assert len(tilemap)==(src.size[0]/BLOCKSIZE)*(src.size[1]/BLOCKSIZE)
        of.write('<map version="1.0" orientation="orthogonal" width="%d" height="%d" tilewidth="%d" tileheight="%d">\n'%\
            (
                src.size[0]/BLOCKSIZE,
                src.size[1]/BLOCKSIZE,
                BLOCKSIZE,
                BLOCKSIZE
            ))
        of.write ('<tileset firstgid="1" name="tilemap" tilewidth="%d" tileheight="%d" spacing="0" margin="0" >\n'%(
            BLOCKSIZE, BLOCKSIZE
            ))
        of.write ('    <image source="%s_tileset.png"/>\n'%name)
        of.write ('</tileset>\n')
        of.write ('<layer name="layer" width="%d" height="%d" >'%(src.size[0]/BLOCKSIZE,src.size[1]/BLOCKSIZE))
        of.write('<data encoding="csv">')
        of.write(",".join("%d"%(i+1) for i in tilemap))
        of.write('</data>\n')
        of.write ('</layer>\n')


        of.write('</map>')



