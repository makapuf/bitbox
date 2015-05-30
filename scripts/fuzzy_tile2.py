#!/usr/bin/python
"Encode as 16x16 palette, lossy, using RLE or not for map, 16 colors per tile"

'TODO : faster encode, encode really as index:11+rle:5 (eol?) 8bpp'
'no dither ?'



from math import sqrt
from itertools import groupby 
import sys

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
out=False # output data 

LAMBDA = 0.2 # quality, lower is better
USE_RLE = False # can use RLE for maps ?
USE_4bits = False 

# grouper par couples

totsize = 0 # keep statistics
BLOCKSIZE = 16

# HIGHER quality : find centroids of squares, THEN reduce (...)

class Encoder : 
    def encode(self,block) : 
        self.decoded = block
        self.size = 0
        # added size, decoded block

    def commit(self,block) : 
        # add data,  really encode
        pass

class Raw(Encoder) : 
    def encode(self,block) : 
        self.decoded = block
        self.size = BLOCKSIZE*BLOCKSIZE/2+32 # new block+ new palette

    def commit(self,block) : 
        tileset.append(block)
        tilemap.append(len(tileset)-1)
        return BLOCKSIZE*BLOCKSIZE/2 # 4 bits per pixel !


class Single(Encoder) : 
    def encode(self,block) : 
        c = tuple(sum(c[band] for c in block)/(BLOCKSIZE*BLOCKSIZE) for band in (0,1,2))
        self.decoded = [c]*BLOCKSIZE*BLOCKSIZE
        self.size = 2 # 1 color = hword
    
# fuzzy palette encode
class FuzzyTile(Encoder) :
    def encode(self,data) : 
        # get closest tile

        # Faster : cut eval if larger than preceding  : bad/good enough
        idx = min(range(len(tileset)),key=lambda x:distance(tileset[x],data))
        #if tilemap and idx!=tilemap[-1] : print tilemap[-1]-idx

        self.idx = idx        
        self.decoded = tileset[idx]  # RLE
        self.size= 2 if not tilemap or (USE_RLE and  tilemap[-1]!=idx) else 0 # RLE ENCODE

    def commit(self,block) : 
        tilemap.append(self.idx)
    def __str__(self) : return 'fuzzy %d %s ..'%(self.idx, tileset[self.idx][:5])



def output_tileset() :
    tileset_image = Image.new("RGB",(8*BLOCKSIZE,(len(tileset)+7)/8*BLOCKSIZE))
    for i,t in enumerate(tileset) :
        imgblock.putdata(tuple((x[0]*C_Fac,x[1]*C_Fac,x[2]*C_Fac) for x in t))
        tileset_image.paste(imgblock,(i%8*BLOCKSIZE,i/8*BLOCKSIZE))
    tileset_image.save(name+'_tileset.png')

C_Fac = 8
a=((1,4),(3,2)) #  1 4 / 3 2 /5
def d(c,i) : 
    x=i%BLOCKSIZE ; y=i//BLOCKSIZE
    return (c+a[y%2][x%2]*C_Fac/5)/C_Fac # a[y,x]

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


def distance(block1,block2, maxdist=9.9e99) : 
    # sum sq difference over each band over each pixel.
    # YUV MODE, Y 3x more important => only Y ?
    yb1 = blocktoYUV(block1)
    yb2 = blocktoYUV(block2)
    #return sum( (c1[0]-c2[0])**2 for c1,c2 in zip(yb1, yb2)) / BLOCKSIZE/ BLOCKSIZE*100. # luminance only
    #return sum(sum((b1-b2)**2) for b1,b2 in zip (c1,c2)) for c1,c2 in zip(block1, block2)) / BLOCKSIZE/ BLOCKSIZE*100. # RGB
    return sum(sum(p*((b1-b2)**2) for b1,b2,p in zip (c1,c2,[3,1,1])) for c1,c2 in zip(yb1, yb2)) / BLOCKSIZE/ BLOCKSIZE*100.


encoders = [ Raw(), FuzzyTile()]

for name in sys.argv[1:] : # for each input file
    print " *** ",name
    stats = {} # method: nb

    tileset = [tuple([(0,0,0)])*BLOCKSIZE*BLOCKSIZE] # only one empty tile
    tilemap = []

    src = Image.open(name+'.png').convert('RGB') # XXX ALPHA
    dst = src.copy()

    encoded_blocks = [] 
    encoded_size = 0
    print '// blocks  , lambda :%.1f, blocksize: %d '%(LAMBDA, BLOCKSIZE)

    for dim in 0,1 : 
        if src.size[dim]%BLOCKSIZE==0 : 
            print 'file %s : %s size not a multiple of %d'%(name,'xy'[dim],BLOCKSIZE)
            continue 

    for y in range(src.size[1]//BLOCKSIZE) : 
        for x in range(src.size[0]//BLOCKSIZE) : 
            imgblock = src.crop((x*BLOCKSIZE,y*BLOCKSIZE,(x+1)*BLOCKSIZE,(y+1)*BLOCKSIZE))
            if USE_4bits : 
                imgblock = imgblock.convert('RGB').convert('P',palette=Image.ADAPTIVE, colors=16).convert('RGB')
            block = tuple( tuple(d(c,i) for c in x) for (i,x) in enumerate(imgblock.getdata())) # keep image in RAM as RGB tuples. 

            # minimize disto + lambda*rate 
            def score(encoder) : 
                encoder.encode(block)
                return distance(encoder.decoded, block) + LAMBDA * encoder.size

            best_encoder= min(encoders, key=score)
            stats[best_encoder]=stats.get(best_encoder,0)+1

            best_encoder.commit(block)

            size = best_encoder.size
            encoded_size += size
            # decode + ouput decoded image ?
            imgblock.putdata(tuple((x[0]*C_Fac,x[1]*C_Fac,x[2]*C_Fac) for x in best_encoder.decoded))

            dst.paste(imgblock,(x*BLOCKSIZE,y*BLOCKSIZE))

        rawsize=src.size[0]*BLOCKSIZE*(y+1)*2
        if y%4==0 : 
            print 'line',y,'/',src.size[1]//BLOCKSIZE,len(tileset), encoded_size,
            print 'raw:', rawsize, 'reduc:',rawsize/encoded_size, stats
            #print 'encoded size',encoded_size
            #for k,v in stats.items() : print k,v

            dst.save(name+'decoded.png')

            output_tileset() # optional ?

    print 'encoded size:',encoded_size, 'reduc : %.1f'%(src.size[0]*src.size[1]*8./encoded_size)
    print stats
    
    totsize += encoded_size

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



print "total",totsize





