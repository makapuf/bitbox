'''
packbits alg. avec transp / blits. individual pixels (no couples), no palette

plain reducing to 15bpp

(16){nfill:7, skip:1, eol:1, ncopy:7}, uint16_t color(si nfill>0), data []


TODO 
----
bin version as parameter : handle direct inclusions
    http://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc/
generate object directly

lossy 
handle couples ?
handle semitransp ?

'''

# 2 8bit headers  : nb skip1:7,  eol : 1; fill:1 ; len :7 
# optimise pour les sprites

# forces ordered dither with a 2x2 matrix on color reduction of 16 or 256; do not reduce further.
# dont try to modify 

out=True
bin_version=True
RLE_threshold=4
RLEMAX = 127


from itertools import groupby 
from collections import defaultdict
import sys
from os.path import isdir, isfile, basename
from glob import glob
from collections import Counter
from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.


def reduce(c) :
    'R8G8B8A8 to A1R5G5B5' # XXX Dither
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

def transparent(c) : 
    return c==0

def runs(line) : 
    "generator, emit runs of either copy or fills or transparents"    
    singles = []
    for c,g in groupby(line) : 
        n = len(tuple(g))
        if transparent(c)  : 
            if singles :
                yield 'c',singles # chunked
                singles=[]
            # max RLE !
            yield 't',n
        elif n>RLE_threshold: 
            if singles : 
                yield 'c',singles 
                singles=[]
            while n!=0 :
                nb = min(RLEMAX,n)
                yield 'f',c,nb
                n-=nb
        else : # singles or not enough
            for i in range(n) : singles.append(c)
    if singles : 
        yield 'c',singles
        singles=[]


class Blit : 
    'a blit has a fill and (then) a copy instruction.'
    def __init__(self) : 
        self.fill_c = 0
        self.fill_n = 0
        self.data = []
        self.eol  = False

    def __str__(self) :
        s= "  PB_H(%d,%d,%d,%d),\t"%(
            self.fill_n,
            0 if self.fill_c else 1,
            len(self.data),
            1 if self.eol else 0
            )

        if self.fill_c : 
            s+='0x%04x,   '%self.fill_c
        s += ''.join("0x%x,"%c for c in self.data)
        return s
    def header(self) : 
        assert self.fill_n<128," %d x %04x"%(self.fill_n,self.fill_c)
        assert len(self.data)<128
        return (self.fill_n)<<9 | (0 if self.fill_c else 1)<<8 | (1 if self.eol else 0) << 7 | len(self.data)

    def as_num(self) : 
        l = [self.header()]
        if self.fill_c : l.append(self.fill_c)
        l += self.data
        return l

def encode(name) : 

    if isdir(name) : 
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

    elif isfile(name+'.png') : 
        filenames = [name+'.png']
        src = Image.open(name+'.png').convert('RGBA') # XXX transp
        frame_h=src.size[1]

    else : 
        raise IOError, "Error : '%s' not found or not a file/dir ! "%name

    data = tuple(reduce(x) for x in src.getdata()) # keep image in RAM as RGBARGBA (2pixels) tuples. 
    w,h=src.size
    singleslen=singlesrun=rlerun=rlelen=0
    
    nb = {}; viewed = set()
    trans = defaultdict(int) # from a to b
    last_method = None

    blits = []
    pos_line =0
    line16_index = [] # byte index of each 16 lines 

    size=0 # byte position 
    for y in xrange(h) :
        # line index
        if y %16 == 0 : 
            line16_index.append(len(blits))

        line=data[y*w:(y+1)*w]

        blits.append(Blit()) # first empty 

        for x in runs(line) : 
            if x[0] == 'c' : 
                # c fills a preceding blit. 
                blits[-1].data = x[1]
                                
            # fills open a new blit
            elif x[0]=='f' : 
                # new one if needed
                if blits[-1].fill_n : 
                    blits.append(Blit()) 
                blits[-1].fill_c=x[1]
                blits[-1].fill_n=x[2]

            else : # trans 
                # new one if needed
                if blits[-1].fill_n : 
                    blits.append(Blit()) 
                blits[-1].fill_n=x[1] 
        
        # end of line

        if not (blits[-1].data or blits[-1].fill_c): 
            if len(blits)==1 or len(blits)>1 and blits[-2].eol : 
                blits[-1].fill_n=0 # no need
            else : # not the only blit : remove it
                del blits[-1]

        # add eol
        blits[-1].eol=True
        trans[x[0],None] += 1 
        last_method=None

    # XXX virer dernieres lignes si vides
    
    size = sum(2+len(b.data)*2 for b in blits)+sum(2 for b in blits if b.fill_c)

    # ---------------------------------------------------------------------------
    of=open(name+'.h','w')

    of.write('''/* 

     {name} 
 -- 
 generated by {progname}, with rle_min={rlemin}
 file(s) : {files}
 {size} bytes ({permeg}/1M, {bpp:0.1f} bpp from 16 bpp), reduction by {reduc:0.1f}
 -- 
 nb blits          : {nblits}, {blitsperline:0.1f}/line
 skips             : {nskips}
 empty fills       : {emptyfills}
 empty copy        : {emptycopy}
 full fills(n=127) : {fullfills}
*/

//.data = either forward(txt) or bin inlucluded or RAM data (dynamic) ...

packbit_rom pb_{name}={{.w = {w}, .h={h}, .frame_h={frame_h}, .idx_len={idx_len}, .data={name}_data}};

'''.format(

    name=name, 
    files=  '\n   '.join(filenames),

    progname = basename(sys.argv[0]),
    rlemin = RLE_threshold,

    size=size,   permeg = 1024*1024/size,   bpp=size*8/(float(w)*h),reduc=2.*w*h/size,
    nblits=len(blits),
    blitsperline=float(len(blits))/h,
    nskips= sum(1 for b in blits if not b.fill_c),
    emptyfills=sum(1 for b in blits if not b.fill_n),
    emptycopy=sum(1 for b in blits if not b.data),
    fullfills=sum(1 for b in blits if b.fill_n==127),
    w=w,h=h,
    frame_h=frame_h,
    idx_len=len(line16_index),
    data_ref='_binary_file_ext_start'
    ))

    if not bin_version and out: 
        of.write('#define PB_H(fill,skip,copy,eol) (fill<<9|skip<<8|eol<<7|copy)\n')
        of.write("%s_data = {\n// -- idx16 \n"%name)
        of.write(','.join('%d'%x for x in line16_index)+',\n// -- blits\n')

        line=0
        for bl in blits[pos_line:] :
            of.write(str(bl)+'\n')
            if bl.eol : 
                line +=1
                of.write('// %d\n'%line)
        of.write('    }\n')

    else : 
        from array import array
        arr=array('H') # arrays of halfword
        arr.extend(line16_index)
        for b in blits : 
            arr.extend(b.as_num())
        arr.tofile(open(name+'.pbt','wb'))

    return size

if __name__=='__main__' :
    if len(sys.argv)==1 : 
        sys.argv += ['hand','fant','ball','snes16']
    elif sys.argv[1] in ('-h','?','--help') : 
        print "usage : %s file1 file2 dir1 dir2 ...\nwhere dir contains 0001.png, 0002.png, ..."

    totsize = 0 # keep statistics
    for name in sys.argv[1:] : # for each input file
        size = encode(name)
        totsize += size
        print "output %s.h : %d bytes"%(name,size)

    print '\n// total size (all files) : %d bytes'%totsize





