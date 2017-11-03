#!/usr/bin/env python
"""
Usage 
=====

- This utility creates sprite .spr files from several png or a tsx (tiled map editor) files
- It creates one animation for a png spritesheet / files or N animations for a tsx.
- You can provide the size of a tile in a tilesheet as filename_16x16.png for a 16 by 16 tile.
- By default the size of a tile is the size of the first png given

Format
=======

The sprite format is the following: (little endian) 

HEADER
    u16 magic : 0xb17b
    u16 width
    u16 height of a frame
    u8  nb of frames
    u8  encoding data code 
        0 : u16 data : data is made as raw u16 pixels
        1 : u8 : data is made of raw u8 blits. No palette expansion is done (can be done at the end)
        2 : cpl : data is made of couples references

    u16 hitbox_x1
    u16 hitbox_y1
    u16 hitbox_x2
    u16 hitbox_y2

    u32 frame_index[nb of frames] : 
        index of frames in the data. 
        0 means start of data, not start of file.
        frames are deduplicated, eg it's not monotonous nor non-unique

PALETTE (only if Data code == cpl)
        u32 palette _length
        u32 palette[palette_length]

DATA : 

    data is made of blits
    blit header : 
        u2 : blit code
        u1 : eol (end of line)
        u5 : length in pixels (even for couples)
    if len = 31 : 
        read u8, add to length
        repeat while byte just read is 255

    blit_code = 0 : 
        skip length pixels
    blit_code = 1 : fill
        read a u8 or u16 depending on encoding_data_code
        duplicate length times, expand palette if cpl mode
    blit_code = 2 : data
        read length pixels of u8 or u16 depending on code. 
    blit_code = 3 : back reference
        read u16 backreference
        blit it as if copy 
"""

import sys
import os
import argparse
import struct 
import array
from PIL import Image # Python Image Library. This can be the pillow friendly, more up to date, fork 
import xml.etree.ElementTree as ET
from itertools import groupby
import re

from utils import *

DEBUG=False
VERBOSE_SPR=False # explicits blits for sprite encoding
MIN_MATCH=2000  # u16 / couples / quads
MIN_FILL =2  # u16 / couples

class Encoder : 
    def __init__(self, frames, hitbox) : 
        # dedup frames
        self.idframes=[] # frame -> unique frame reference        
        self.unique_frames = [] # list of unique frames

        for i,img in enumerate(frames) : 
            if img not in self.unique_frames : self.unique_frames.append(img)
            self.idframes.append(self.unique_frames.index(img))

        self.src = stack_images_vertically(self.unique_frames)

        self.frm_h = self.unique_frames[0].size[1]

        self.hitbox = hitbox
        self.palette=None

    def cutlines(self) : 
        """cut an image into lines as blits. 
        blit = 
            int size, None : skip
            int size, [rgba value, ...]
        """
        self.blits = []
        avoided=0

        imgdata = tuple(self.src.getdata())
        w,h = self.src.size
        for y in range(h) : 
            line = imgdata[y*w : (y+1)*w]
            for transp, tr in groupby(line, lambda t:t[3]<127) : 
                if transp : 
                    self.blits.append((sum(1 for _ in tr),None,False))
                else : 
                    pix = list(tr)
                    self.blits.append((len(pix),pix, False))
                if self.blits[-1][0]>32 and self.blits[-1][0]<63 : avoided -=1
            lastblit = self.blits.pop()            
            # if it was a skip and there was a data, non-eol before, put the eol at the preceding data point
            if lastblit[1]==None and self.blits and self.blits[-1][2] == False : 
                lastblit = self.blits.pop()   
                avoided +=1
            self.blits.append((lastblit[0],lastblit[1],True))
        print 'avoided with eol',avoided


    def packbits(self):
        """group data into repeats or runs of data.
        will replace (n,[data],eol) blits to (n,element,eol) blits with element same as data.
        """
        PPE = 2 if self.datacode == DATA_cpl else 1 # pixels per data element
        nblits =[]
        for nbpx,bl,eol in self.blits: 
            # encode bl into a series of blits, add them to nblits 
            if bl==None : 
                nblits.append((nbpx,bl,eol))
                continue

            subbl = []
            for c,it in groupby(bl) : 
                n = sum(1 for _ in it)
                if n>=MIN_FILL : 
                    subbl.append((n*PPE,c,False)) # eol forced to false
                else : 
                    if subbl and type(subbl[-1][1])==list : 
                        subbl[-1] = (subbl[-1][0]+n*PPE, subbl[-1][1] + [c]*n, False)
                    else :
                        subbl.append((n*PPE,[c]*n,False))
            # only last one will be eol, if any
            p = subbl.pop()
            subbl.append((p[0],p[1],eol))

            nblits += subbl

            # correct last one if was an odd element
            if self.datacode == DATA_cpl and nbpx%2 : nblits[-1] = (nblits[-1][0]-1, nblits[-1][1],nblits[-1][2])
        self.blits = nblits

    def strblits(self) : 
        "encode literal / fill blits to strings"
        t = 'H' if self.datacode == DATA_u16 else 'B'
        binblits = []
        for sz,bl,eol in self.blits : 
            if type(bl)==list : 
                binblits.append((sz,array.array(t,bl).tostring(),eol))
            else  : # none or int
                binblits.append((sz,bl,eol))

        self.blits = binblits

    def generate_bin(self) : 
        "pack all blits, compute binary + frame_index"
        # prepare blits for the format : remove skip with eol if preceding, split into 32-width
        newblits = []
        prev_eol = True

        CODE_SKIP = 0
        CODE_FILL = 1
        CODE_DATA = 2
        CODE_REF  = 3

        print ' ** stats'
        print len(self.blits),'blits'
        print sum(1 for bl in self.blits if bl[1]==None ),'skips'
        print sum(1 for bl in self.blits if type(bl[1])==int ),'fills'
        print sum(1 for bl in self.blits if type(bl[1])==str ),'copy, size=',sum(len(x) for x in self.blits if type(x[1]==str))
        print sum(1 for bl in self.blits if bl[2]),'lines'
        uniq_data= set(x[1] for x in self.blits if type(x[1])==str)
        print len(uniq_data),'different blits size =',sum(len(x) for x in uniq_data)

        s = ""
        self.frame_index = [0]
        y=0
        for n,bl,eol in self.blits : 
            if type(bl)==int : 
                code = CODE_FILL
                data = struct.pack('H' if self.datacode==DATA_u16 else 'B',bl) # add u16 or u8
            elif bl==None :
                code = CODE_SKIP
                data = ''
            else : 
                code = CODE_DATA
                data = bl
                if len(bl)>MIN_MATCH and bl in s : 
                    idx = len(s)-s.rindex(bl)
                    if idx < 65536 : 
                        code= CODE_REF
                        data= chr (idx>>8) + chr(idx & 0xff)

            # blit header
            s += chr(code<<6 | ((1<<5) if eol else 0) | min(n,31) )
            n-=31
            while n >= 255 : 
                s += chr(255)
                n -= 255
            if n >=0 : 
                s += chr(n)

            # data
            s += data

            if eol  : 
                y += 1 
                if y%self.frm_h==0 : 
                    self.frame_index.append(len(s))

        self.bindata = s
        print '%d bytes, %.1f bpp'%(len(self.bindata), 8.*len(self.bindata) / (self.src.size[0] * self.src.size[1]))

    def write_header(self,of) : 
        w,h = self.src.size
        of.write(struct.pack('HHHBB',0xb17b,w,self.frm_h,len(self.idframes),self.datacode)) # this is the number of frame references
        print >>sys.stderr,'hitbox:',self.hitbox
        of.write(struct.pack('4H',*self.hitbox))
        array.array('I', [self.frame_index[id] for id in self.idframes]).tofile(of)

    def write_palette(self,of) : 
        array.array('I', 
            [len(self.palette)]+[rgba2u16(*c[4:])<<16 | rgba2u16(*c[:4]) for c in self.palette ]
        ).tofile(of)


    def prepare(self) : 
        self.cutlines() # n, None / [rgba...] blits , eol 
        self.encode()   # n, none / [rgba u16/8/cpl refs], eol, creates self.palette
        self.packbits() # n, None, color, [rgba] , eol

        self.strblits() # n, None, color, string, eol

        self.generate_bin() # self.index, self.bindata (including palette if code = cpl)

        
class Encoder_u16 (Encoder) : 
    datacode = DATA_u16
    def encode(self) : 
        "create blits with RGBA to blits with u16"
        self.blits = [ (bl[0],[rgba2u16(*c) for c in bl[1]], bl[2]) if type(bl[1])==list else bl for bl in self.blits ]

class Encoder_u8 (Encoder)  : 
    datacode = DATA_u8
    def __init__(self, frames, palette_type, hitbox) : 
        if palette_type=='MICRO' : 
            self.palette=gen_micro_pal()
        else : 
            self.palette=Image.open(palette)
        Encoder.__init__(self,frames,hitbox)

    def encode(self) : 
        # convert to this 8bpp palette, keeping transparency 
        img=quantize_alpha(self.src,self.palette)
        if DEBUG: img.save('_debug.png')
        data = list(img.getdata())
        transparent= img.info['transparency']
        cut_lines
        
        return data,self.palette

class Encoder_cpl (Encoder) : 
    datacode = DATA_cpl

    def encode(self) : 
        # get all couples, return blits as : int nbpixels + None or (ref cpls1, ...) ; palette couples 
        # encode couples
        w,h = self.src.size

        cplblits = [] # blits as couples, maybe increased to even pixels
        for size_px, blit,eol in self.blits :
            if blit == None : 
                cplblits.append((size_px, blit,eol))
                continue
            if size_px%2 : blit.append(blit[-1]) # fixme find closest couple ?
            cpl_blit = [tuple(blit[i]+blit[i+1]) for i in range(0,len(blit),2)]
            cplblits.append((size_px,cpl_blit,eol))

        couples = sum((x for sz,x,eol in cplblits if x!=None),[])
        target_nb_couples = min(255,len(couples)/16)

        print ' *',len(couples),"couples,",len(set(couples)),"different, to",target_nb_couples
        self.palette, invpal = quantize_couples(couples,target_nb_couples)

        # now express blits as couples references
        self.blits = []
        for bl in cplblits : 
            if type(bl[1]) != list : 
                self.blits.append(bl)
            else : 
                self.blits.append((bl[0],[invpal[x] for x in bl[1]],bl[2]))


def encode_spr(frames, palette_type, hitbox, outfile) : 
    if palette_type == None : 
        e = Encoder_u16(frames, hitbox)
    elif palette_type == 'COUPLES' : 
        e = Encoder_cpl(frames, hitbox)
    else : 
        e = Encoder_u8 (frames, palette_type, hitbox)

    e.prepare()
    with open(outfile,'wb') as of : 
        e.write_header(of)        
        if e.palette : 
            e.write_palette(of)
        of.write(e.bindata)

    #frame_index, binary = encode_lines(data, w,h,frm_h, transparent, datacode)

def read_tsx_states(ts) : 
    """read sprite animations, types and list used tiles from tsx file"""
    for tile in ts.findall('tile') : 
        tid = int(tile.get('id')) # local id
        typename = tile.get('type')
        if typename==None : continue

        # get animation tiles
        anim_elt = tile.find('animation')
        if anim_elt != None : # animated tile
            frames = [ int(frame_elt.get('tileid')) for frame_elt in anim_elt.findall('frame') ] # XX also duration ?
        else : # no animation : animation is made from only one tile : the tile id itself
            frames = [ tid ]

        # find hitbox or set it full
        try : 
            hit_elt = tile.find('objectgroup').find('object') # take first hit object  <object id="1" x="10.75" y="15.125" width="10.375" height="11.375"/>
            hitbox=(
                int(float(hit_elt.get('x'))),
                int(float(hit_elt.get('y'))),
                int(float(hit_elt.get('x'))+float(hit_elt.get('width'))),
                int(float(hit_elt.get('y'))+float(hit_elt.get('height')))
            )
        except AttributeError,e : 
            hitbox=(0,0,int(ts.get('tilewidth')),int(ts.get('tileheight')))
        yield typename, frames, hitbox
        
# --- High level functions

def png2spr(infiles, outfile, palette_type, size) : 
    if size : 
        gw,gh = (int(x) for x in size.split("x"))
    else : 
        # try to get from filename example mysprite_14x8.png
        m= re.search(r'_(\d+)x(\d+).png$',infiles[0].name)
        if m==None :
            gw,gh = Image.open(infiles[0]).size
        else : 
            gw = int(m.groups(1)[0])
            gh = int(m.groups(1)[1])
            ggw,ggh = Image.open(infiles[0]).size
            if gw > ggw or gh > ggh : usage('provided frame size %d,%d in filename too big'%(gw,gh))

    print >>sys.stderr, " - generating sprite file %s from %s frame size : %dx%d"%(outfile, ",".join(x.name for x in infiles), gw, gh)

    srcs = []
    for file_in in sorted(infiles, key=lambda x:x.name) : 
        img = Image.open(file_in).convert('RGBA')
        srcs += cut_image(img,gw,gh)

    hitbox = (0,0,gw,gh)
    encode_spr(srcs,palette_type, hitbox, outfile)
   

def tsx2sprN(tsxfile,  palette, outdir) : 
    "one tsx to N sprite files."
    print >>sys.stderr, "generating sprite files from %s"%tsxfile
    ts=ET.parse(tsxfile).getroot()
    name = ts.get('name')

    image = ts.find('image').get('source')
    ts_w = int(ts.get('tilewidth' ))
    ts_h = int(ts.get('tileheight'))
    imgsrc=abspath(tsxfile.name, image)
    frames=cut_image(Image.open(imgsrc).convert('RGBA'),ts_w, ts_h)

    for typename, frames_id, hitbox in read_tsx_states(ts) :
        filename = os.path.join(outdir, name+'_'+typename+'.spr')
        print >>sys.stderr, 'exporting', filename
        encode_spr([frames[i] for i in frames_id], palette, hitbox, filename)

# --- Main : commandline parsing

if __name__=='__main__' : 
    parser = argparse.ArgumentParser(description='Process files for bitbox graphical library.')
    parser.add_argument('file_in', metavar='file',nargs='+',help='input files (tsx,tmx,png)',type=argparse.FileType('rb')) 
    parser.add_argument('-o','--output',  help='output file (spr, )', required=True) 
    parser.add_argument('-p','--palette', help='palette name/file. Can use a .png file (255 colors max + transp) or MICRO, or COUPLES')
    parser.add_argument('-s','--size',    help='(for spr) WxH size in pixels of the sprite')
    parser.add_argument('--min_match',    help='minimum match', type=int, default=4)
    parser.add_argument('--min_fill',     help='minimum fill',  type=int, default=4)

# fixme autocrop ? separate script
    args = parser.parse_args()

    def usage(str) : 
        print >>sys.stderr, "Usage error :", str
        sys.exit(2)

    first_name,first_ext = args.file_in[0].name.rsplit('.',1)

    if any(f.name.rsplit('.',1)[1]!=first_ext for f in args.file_in) : 
        usage('all input files must be the same type')

    print >>sys.stderr, "// %s with args:"%os.path.basename(sys.argv[0]),args

    MIN_MATCH = args.min_match
    MIN_FILL  = args.min_fill

    # dispatch from first file type
    if first_ext == 'png' : 
        out_ext = args.output.rsplit('.',1)[1]
        if out_ext == 'spr' : 
            png2spr(args.file_in, args.output, args.palette, args.size)
        else: 
            usage("png files can only generate .spr files")

    elif first_ext == 'tsx' : 
        if len(args.file_in) != 1 : 
            usage("can process only one input tsx file")
        # disallow format, size
        if not os.path.isdir(args.output) : usage("output must be a directory for tsx files")
        tsx2sprN(args.file_in[0],args.palette, args.output)
