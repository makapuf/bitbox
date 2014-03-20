'''
packbits avec transp + palette / blits. individual pixels (no couples).
plain reducing to 15bpp / 8bits palette
better results with preprocessing (16/256 palette after 15bit reduction, dithering, ...) 
dithering is not well compressed however
16 lines byte index (can be used to handle multiframe)
handle pixel output as 4bpp if enabled - seulement ds blits
'''

'''
TODO 
----
decompress 4bits / tell bpp  or nb colors !

handle couples ?
handle multiframes ; in subdir : (abc/XXXX.png) or in abc_XXXX.png : concat vertically, set frame size (assumes same vsize)
handle transparent color

packbit+mask ? premul

'''

from itertools import groupby 
import sys

# 2 8bit headers  : nb skip1:7,  eol : 1; fill:1 ; len :7 
# optimise pour les sprites

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
# forces ordered dither with a 2x2 matrix on color reduction of 16 or 256; do not reduce further.
# dont try to modify 

out=True
plus= False  # original data for debugs

RLE_threshold=7
RLEMAX = 127

# print '#include <stdint.h>'
# print '// RLE min : ',RLE_threshold

if len(sys.argv)==1 : 
    for s in 'fant','ball_0001','anim_mechant1','anim_vaisseau256','logo2','rect6352','freestyle','arbres_7':#,'bg0001' 'firemen_bg_15','firemen_bg','freestyle','logo2','freestyle2','rect6352','snes16',: # 'sunset',
        sys.argv.append(s)

totsize = 0 # keep statistics
a=b=m=0
from collections import Counter

def reduce(c) :
    'R8G8B8A8 to A1R5G5B5' # XXX Dither
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 


def palettize(im,col) : 
    # PIL complains if you don't load explicitly
    im.load()
    im=im.convert('RGBA')

    # Get the alpha band
    alpha = im.split()[-1]

    imp = im.convert('RGB').convert('P', palette=Image.ADAPTIVE, dither=Image.NONE, colors=col)

    # Set all pixel values below 128 to 255,
    # and the rest to 0
    mask = Image.eval(alpha, lambda a: 255 if a <=128 else 0)

    # Paste the color of index 255 and use alpha as a mask
    imp.paste(255, mask)
    imp.info['transparency']=chr(255)

    return imp

def transparent(c) : 
    global transparent_color
    return c==transparent_color

def runs(line) : 
    "generator, emit runs of either copy or fills or transparents"    
    singles = []
    for c,g in groupby(line) : 
        n = len(tuple(g))
        if transparent(c)  : 
            if singles :
                yield 'c',singles # chunked
                singles=[]
            yield 't',n
        elif n>RLE_threshold: 
            if singles : 
                yield 'c',singles 
                singles=[]
            # RLE max ..
            while n>0 :
                nb = min(RLEMAX,n)
                yield 'f',c,nb
                n-=nb
        else : # continue appending
            for i in range(n) : singles.append(c)
    if singles : 
        yield 'c',singles
        singles=[]


class Blit : 
    def __init__(self, bpp) : 
        self.bpp = bpp
        self.skip =  0
        self.data =  None
        self.color= None
        self.len   = 0
        self.eol   = False
    
    def set_data(self,data) : 
        assert self.color==None
        self.data=data
        self.len = len(data)

    def set_fill(self, c,len):
        self.len=len
        self.color=c

    def __str__(self) : 
        " {skip:7;eol:1;fill:1;len:7;uint8_t pixels[];} packbit_record;"
        s= "  (%d<<1 | %d), (%d<<7 | %d) ,"%(self.skip,1 if self.eol else 0, 1 if self.color is not None else 0,  self.len)
        if self.color is not None: 
            s += "0x%x,"%self.color
        else : 
            if self.data is not None : 
                assert len(self.data)==self.len," %d != %d"%(len(self.data),self.len)
                if self.bpp==8 : 
                    data = self.data
                else : 
                    if len(self.data)%2 : self.data.append(0)
                    data = [self.data[i]<<4 | self.data[i+1] for i in range(0,len(self.data),2)]

                s += ''.join("0x%x,"%c for c in data)
        return s


def within (a,b) : 
    "true if a within b"
    # here at least 4 start identical.
    return tuple(a)==tuple(b[:len(a)]) or tuple(a)==tuple(b[-len(a):])

for name in sys.argv[1:] : # for each input file
    print >>sys.stderr,"output %s.h"%name
    sys.stdout=open(name+'.h','w')
    print '\n// ***',name
    src = Image.open(name+'.png') # XXX transp

    # convert to palette
    if src.mode == 'P': 
        col= max(src.getdata())+1
        src8bpp=src
    else : 
        col = min(len(set(src.getdata())),255)+1
        src8bpp=palettize(src,col) 

    try : 
        transparent_color = ord(src8bpp.info['transparency'])
    except KeyError : 
        transparent_color = None

    pal=src8bpp.getpalette()[:3*col]
    # sometimes they are too big
    print "static const uint16_t %s_palette[%d]={"%(name,len(pal)/3)+','.join('0x%x'%reduce((pal[pos:pos + 3]+[255])) for pos in xrange(0, len(pal), 3))+'};'

    data = tuple(src8bpp.getdata()) # keep image in RAM as RGBARGBA (2pixels) tuples. 

    bpp=4 if col <= 16 else 8
    print "// %s_bpp =%d; // %d colors after color reduction"%(name, bpp, len(set(pal)))

    # to couples of palette data
    couples = [data[pos:pos + 2] for pos in xrange(0, len(data), 2)]

    # rebuild output image
    src8bpp.save(name+'_out.png')
    #print "saved ",name+'_out.png'

    w,h=src.size
    singleslen=singlesrun=rlerun=rlelen=0
    
    if out : print 'static const uint8_t %s_data[] = { '%name
    
    nb = {}; viewed = set()

    blits = []
    pos_line =0
    line16_index = [] # byte index of each 16 lines 

    size=0 # byte position 
    for y in xrange(h) :
        # line index
        if y %16 == 0 : 
            print '// --- line %d ------ '%y
            line16_index.append(size)

        line=data[y*w:(y+1)*w]
        blits.append(Blit(bpp))
        if plus : print '//',len(line),line
        for x in runs(line) : 
            nb[x[0]] = nb.get(x[0],0)+1

            if x[0] == 'c' : 
                # try to reuse a past element ?
                size += 2+len(x[1]) if bpp==8 else (len(x[1])+1)/2 # header
                # size += len(x[1]) if (bpp==8) else len(x[1])/2
                blits[-1].set_data(x[1])
                blits.append(Blit(bpp)) # new one

                
                if any(within(x[1], vv) for vv in viewed) : 

                    # view : size economisee si ref au lieu de plain
                    nb['view'] = nb.get('view',0)+(len(x[1]) if (bpp==8) else len(x[1])/2 )
                else : 
                    viewed.add(tuple(x[1]))
                
                
            elif x[0]=='f' : 
                blits[-1].set_fill(x[1],x[2])
                blits.append(Blit(bpp)) # new one
                #size += 4 if (bpp==8) else 3 # size : 2B header + 2 pixels
                size += 3

            else : # trans : already taken in 2b headers
                blits[-1].skip=x[1]
        
        # end of line
        # remove last blit if empty
        if len(blits)>1 and (not blits[-2].eol) and blits[-1].len == 0 : 
            del blits[-1]

        # add eol
        blits[-1].eol=True

        # XXX virer dernieres lignes
        if out:
            # print line (ie all blits since last line)
            for bl in blits[pos_line:] : print bl
            print

            pos_line = len(blits)

    if out:print '};'

    size += len(pal)/3 # palette
    print 'static const uint16_t %s_idx16[%d] = {'%(name,len(line16_index))+','.join('%d'%x for x in line16_index)+'}; // indices of each 16 lines (0,16,32,...)'
    size += 2*len(line16_index)
    print 'const packbit_rom pb_%s={.w = %d, .h=%d, .data=%s_data, .idx16=%s_idx16, .palette=%s_palette};'%(name,w,h,name,name,name)

    print '//',nb
    print '// %d bytes (%d/1M, %.1f bpp from %d bpp), reduction by %.1f'%(size,1024*1024/size,size*8/(float(w)*h),bpp,2*float(w)*h/size)
    if rlerun : 
        print '// %d fills, %d len, avg %.1f'%(rlerun, rlelen,float(rlelen)/rlerun)
    if singlesrun : 
        print '// %d copy, %d len, avg %.1f'%(singlesrun, singleslen,float(singleslen)/singlesrun)

    totsize += size

print >>sys.stderr,'\n// total size (all files) : %d bytes'%totsize

