#! /usr/bin/python2

# http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=00152671
# http://en.wikipedia.org/wiki/Block_Truncation_Coding
# interpolation use ? transp 4bits hi/lo/mid/trans

# 2bpp coding in 4x4 blocks, 15bit data (32bits / block), palettized 


# palettize, cut in 4x4 blocks, sample <> moyenne luminance, get 2 colors
# decompression : use load GE + SEL puis depalettize ? ou inverse


# other possibility : 4bpp, 32 bits color = 4x 8bpp index?

'''TODO : 
transparency as (skip:8,blit:7,eol:1)
sound !

multifiles + temporal dither + align all at 512b 
'''


import sys, argparse
from glob import glob
from PIL import Image 
# using the PIL library, maybe you'll need to install it. python should come with it.
import array
from os.path import isdir, isfile

from collections  import Counter

BLOCKSIZE = 4

if len(sys.argv)==1 : 
    sys.argv += glob('in/*.png')


def d(c,i) : 
    a=((1,4),(3,2))
    # c is 0-255 , outputs to 0-31 
    x=i%BLOCKSIZE ; y=i//BLOCKSIZE    
    return int(c/255.*31.+a[y%2][x%2]/5.)

def lumi(block) : return [0.299 * r + 0.587 * g + 0.114 * b for r,g,b in block ] # Y,U,V

def avg(l) : return sum(l)/float(len(l))
def stddev(l,avg) : return sum(((x-avg)*(x-avg)) for x in l)/float(len(l))

def quantize_colors(cols) : 
    colors=[cols]   
    if len(colors[0])<=255 : 
        print "just keeping %d colors"%len(colors[0])
        palette = list(colors[0])
        invpal = dict((c,n) for n,c in enumerate(palette))
    else : 
        while len(colors)<256 : 
            # find id of largest set  
            cid = max(range(len(colors)),key=lambda i:len(colors[i]))
            colorset =colors[cid] 
            
            # choix de l'axe : max d'ecart type 
            avgs = [avg([c[i] for c in colors[cid]]) for i in range(3)]
            stdv = [stddev([c[i] for c in colors[cid]],avgs[i]) for i in range(3)]
            # get index of max
            axis = max(range(len(stdv)), key=lambda x:stdv[x])

            # couper a la moyenne : enlever & remettre 2 decoupes        
            del colors[cid]
            colors.append(set(c for c in colorset if c[axis] > avgs[axis]))
            colors.append(set(c for c in colorset if c[axis] <= avgs[axis]))

        # now make an invert mapping (asserts not too big !) from mean
        palette=[]
        invpal = {}
        for cl in colors :
            acl=tuple(int(avg([c[i] for c in cl])) for i in range(3))
            if acl not in palette : 
                palette.append(acl)

            for c in cl : 
                invpal[c]=palette.index(acl) # takes first if already in palette
    
    palette += [(0,0,0)]*(256-len(palette)) # pad to 256 values    

    assert all( all(x<256 for x in c) for c in palette),[c for c in palette if any(x>=256 for x in c)]

    invpal[None]=0
    return palette,invpal



def encode_plain(src, dither):
    '''in : source image + dither bool, 
       out : [encoded_block1, encoded_block2] 
             where encoded_block = (col1 col2, u16 pixels)
    '''

    dither_array = ((1,4),(3,2))
    encoded_blocks = [] 
    for y in range(src.size[1]//BLOCKSIZE) : 
        for x in range(src.size[0]//BLOCKSIZE) : 
            imgblock = src.crop((x*BLOCKSIZE,y*BLOCKSIZE,(x+1)*BLOCKSIZE,(y+1)*BLOCKSIZE))
            if dither :
                block = tuple( 
                    tuple(
                        d(c,i) 
                        for c in x) 
                    for (i,x) in enumerate(imgblock.getdata())) # keep image in RAM as RGB tuples. 
            else : 
                block = tuple( tuple(c*31/255 for c in x) for x in imgblock.getdata()) # keep image in RAM as RGB tuples. 
                

            yuvblock = lumi(block)
            avg_y = sum(yuvblock)/float(BLOCKSIZE*BLOCKSIZE) 
            hipix = [b for b,yb in zip(block,yuvblock) if yb>=avg_y]
            lopix = [b for b,yb in zip(block,yuvblock) if yb<avg_y]
            # take average of hi pixels 
            avghi = tuple(int(round(sum(b[i] for b in hipix)/float(len(hipix)))) for i in range(3)) if hipix else None
            if avghi!=None : assert all(x<256 for x in avghi),(avghi,hipix,list(enumerate(imgblock.getdata())))


            avglo = tuple(int(round(sum(b[i] for b in lopix)/float(len(lopix)))) for i in range(3)) if lopix else None
            if avglo!=None : assert all(x<256 for x in avglo),(avglo,lopix)

            # decoded = tuple(avghi if y>=avg_y else avglo for y in yuvblock)
            encoded = sum(1<<n for n,(b,yu) in enumerate(zip(block,yuvblock)) if yu>=avg_y)
            encoded_blocks.append((avghi,avglo,encoded))
    return encoded_blocks

def encode_numpy(src, dither):
    '''in : source image + dither bool, 
       out : [encoded_block1, encoded_block2] 
             where encoded_block = (col1 col2, u16 pixels)
    '''
    
    dither_array = ((1,4),(3,2))
    encoded_blocks = [] 
    for y in range(src.size[1]//BLOCKSIZE) : 
        for x in range(src.size[0]//BLOCKSIZE) : 
            imgblock = src.crop((x*BLOCKSIZE,y*BLOCKSIZE,(x+1)*BLOCKSIZE,(y+1)*BLOCKSIZE))
            if dither :
                block = tuple( 
                    tuple(
                        d(c,i) 
                        for c in x) 
                    for (i,x) in enumerate(imgblock.getdata())) # keep image in RAM as RGB tuples. 
            else : 
                block = tuple( tuple(c*31/255 for c in x) for x in imgblock.getdata()) # keep image in RAM as RGB tuples. 
                

            yuvblock = lumi(block)
            avg_y = sum(yuvblock)/float(BLOCKSIZE*BLOCKSIZE) 
            hipix = [b for b,yb in zip(block,yuvblock) if yb>=avg_y]
            lopix = [b for b,yb in zip(block,yuvblock) if yb<avg_y]
            # take average of hi pixels 
            avghi = tuple(int(round(sum(b[i] for b in hipix)/float(len(hipix)))) for i in range(3)) if hipix else None
            if avghi!=None : assert all(x<256 for x in avghi),(avghi,hipix,list(enumerate(imgblock.getdata())))


            avglo = tuple(int(round(sum(b[i] for b in lopix)/float(len(lopix)))) for i in range(3)) if lopix else None
            if avglo!=None : assert all(x<256 for x in avglo),(avglo,lopix)

            # decoded = tuple(avghi if y>=avg_y else avglo for y in yuvblock)
            encoded = sum(1<<n for n,(b,yu) in enumerate(zip(block,yuvblock)) if yu>=avg_y)
            encoded_blocks.append((avghi,avglo,encoded))
    return encoded_blocks


def decode_image(size, encoded_blocks,encoded_palette,filename) : 
    # decode palette
    palette = [(((x>>10)&0x1f)*8,((x>>5)&0x1f)*8,(x&0x1f)*8) for x in encoded_palette] # low Q *8 !
    dst = Image.new('RGB',size) # XXX A ?
    stride = (size[0]+BLOCKSIZE-1)//BLOCKSIZE
    for n,block in enumerate(encoded_blocks) : 
        bx = n%stride
        by = n//stride
        c1 = palette[block>>24]
        c2 = palette[(block>>16)&0xff]

        for i in range(BLOCKSIZE*BLOCKSIZE) :
            x = bx*BLOCKSIZE + i %BLOCKSIZE
            y = by*BLOCKSIZE + i//BLOCKSIZE
            try : 
                dst.putpixel((x,y),c1 if block&(1<<i) else c2)
            except IndexError,e : 
                print size,x,y,i
                raise
    dst.save(filename)
  

def encode_file(name, options) :
    print " *** ",name
    if isdir(name) : # not always ...
        filenames = sorted(glob(name+'/????.png'))
        print "reading directory",name,':',', '.join(filenames)
        imgs = [Image.open(fn).convert('RGBA') for fn in filenames]
        if not imgs : raise IOError, 'No frame images Found in subdirectory'
        if not all(img.size==imgs[0].size for img in imgs) :
            print "Warning : frames not all the same sizes  : ",[i.size for i in imgs]

        w = max(img.size[0] for img in imgs)
        h = sum(img.size[1] for img in imgs)
        
        frame_h = imgs[0].size[1]

        src = Image.new('RGBA',(w,h))
        y = 0
        for i in imgs:
            src.paste(i, (0,y))
            y += i.size[1]

    elif isfile(name) : 
        filenames = [name]
        src = Image.open(name).convert('RGBA') # XXX transp
        frame_h=src.size[1]

    src = Image.open(name).convert('RGB') # XXX ALPHA
    dst = src.copy()


    # resize image if needed 
    for dim in 0,1 : 
        if src.size[dim]%BLOCKSIZE!=0 : 
            print 'file %s : %s size %d not a multiple of %d, will be cropped '%\
                (name,'xy'[dim],src.size[dim],BLOCKSIZE)            
    src=src.crop((0,0,src.size[0]//BLOCKSIZE*BLOCKSIZE,src.size[1]//BLOCKSIZE*BLOCKSIZE))

    # extract colors
    encoded_blocks = encode_plain(src, options.dither)
    cols=set()
    for c1,c2,_ in encoded_blocks : 
        cols.add(c1)
        cols.add(c2)
    if None in cols : cols.remove(None)

    palette, invpal = quantize_colors(cols)
    assert len(palette)==256,len(palette)

    encoded_header = array.array('I',src.size)
    encoded_pal = array.array('H',(( (r<<10) | (g << 5 ) | b ) for r,g,b in palette))  # encode palette
    
    encoded_u32 = array.array('I',((invpal[c1]<<24) | (invpal[c2]<<16) | encoded for c1,c2,encoded in encoded_blocks))
    assert encoded_pal.itemsize==2
    assert encoded_u32.itemsize==4

    if options.test :
        decode_image(src.size, encoded_u32,encoded_pal,name[:-4]+'out.png')

    if not(options.no_out) : 
        of=name[:-4]+'.btc'
        print "writing",of
        outfile = open(of,'wb')
        encoded_header.tofile(outfile)
        encoded_pal.tofile(outfile)
        encoded_u32.tofile(outfile)
        if (options.pad) : 
            outfile.write('*'*(-outfile.tell()%512))
        outfile.close()


    #stats
    encoded_size = len(encoded_blocks)*4+len(palette)*2
    rawsize=src.size[0]*src.size[1]*2
    print 'raw:', rawsize,'size',encoded_size,'reduc: %.1f'%(float(rawsize)/encoded_size)



parser = argparse.ArgumentParser(description='Process some graphical files and renders them as btc files')
parser.add_argument('filename', metavar='F', type=str, nargs='+',
                   help='a graphical file')
parser.add_argument('-d','--dither',action='store_true',help='dither data')
parser.add_argument('-t','--test', action='store_true', help='produce test image')
parser.add_argument('-p','--pad', action='store_true', help='pad to 512B limit')
parser.add_argument('-n','--no-out', action='store_true', help='do not produce a btc file')

opts= parser.parse_args()

print opts
for name in opts.filename : # for each input file
    encode_file(name, opts)
