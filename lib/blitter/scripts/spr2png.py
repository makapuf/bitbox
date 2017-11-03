#!/usr/bin/env python

'''
spr2png.py : simple decoder for spr format to single png vertical strip
    decode duplicated frames (which are deduplicated in format)
    you can set TYPECOLOR to True to replace each blit type with a color instead of original pixels
'''

"""
TODO
   really copy pixels for refs instead of just blitting constant pixels
"""
from PIL import Image
import struct 
from utils import u162rgba
import sys 

CODE_SKIP = 0
CODE_FILL = 1
CODE_DATA = 2
CODE_REF  = 3

DATA_u16 = 0
DATA_u8  = 1
DATA_cpl = 2

DEBUG = False
TYPECOLOR = False # replace blits by a color for their type

class Sprite : 
    def __init__(self, f) : 
        self.f = open(f,'rb')
        self.palette = None
        self.parse_header()
        self.read_data = [self.read_data_u16,None,self.read_data_cpl][self.datacode]

    def read_data_u16(self,nb) : 
        return [u162rgba(c) for c in struct.unpack('%dH'%nb, self.f.read(nb*2))]

    def read_data_cpl(self,nb) : 
        data = ()
        for i in range((nb+1)/2) : 
            c = ord(self.f.read(1))
            data += self.palette[c]

        # adjust for last one if odd
        if nb%2 : 
            data = data[:-1]
        return data

    def parse_header(self) : 
        magic,self.width,self.height,frames, self.datacode = struct.unpack('HHHBB', self.f.read(8))
        assert magic == 0xb17b
        self.hitbox = struct.unpack ('4H',self.f.read(8))
        self.frames_index = struct.unpack('%dI'%frames, self.f.read(4*frames))
        # now read palette if needed
        if self.datacode == DATA_cpl : 
            s= self.f.read(4)
            palette_len = struct.unpack('I',s)[0]
            palette = [u162rgba(x) for x in  struct.unpack('%dH'%palette_len*2,self.f.read(palette_len*4))]
            self.palette = [(palette[2*i],palette[2*i+1]) for i in range(palette_len)]

    def read_blit(self) : 
        c = ord(self.f.read(1))
        blit_code = c>>6
        blit_eol = c & 1<<5 == 32
        blit_len = c & 31
        # to optimize away if not needed (total width<32 par ex.)
        if blit_len==31 :
            while True : 
                c = ord(self.f.read(1))
                blit_len += c
                if c<255 : break

        yield blit_code, blit_len, blit_eol

    def unpack(self) : 
        self.img = Image.new('RGBA',(self.width,self.height*len(self.frames_index)))
        data = self.img.load()
        file_start = self.f.tell()

        # frame by frame, seek to position first (duplicated frames !)
        for frame_id, frame_idx in enumerate(self.frames_index) : 
            y=0 ; x=0
            # print 'frame',frame_id,'-',frame_idx
            self.f.seek(file_start+frame_idx)
            while y < self.height : 
                bc,bl,beol = next(self.read_blit())
                if DEBUG : 
                    print ['skip','fill','data','copy'][bc],bl,'eol:', beol,'pos:',x,y

                if bc==CODE_DATA : 
                    pixels = self.read_data(bl)
                    for i,color in enumerate(pixels) : 
                        if TYPECOLOR : color = (255,0,0,255)  
                        data[x+i,y+frame_id*self.height] = color

                elif bc==CODE_FILL : 
                    if self.datacode==DATA_u16 : 
                        color = u162rgba(struct.unpack('H',self.f.read(2))[0])                    
                        if TYPECOLOR : color = (0,0,255,255)  
                        for i in range(bl) : data[x+i,y+frame_id*self.height]=color
                    elif self.datacode == DATA_u8 : 
                        color = struct.unpack('B',self.f.read(1))[0]
                        if TYPECOLOR : color = (0,0,255,255)  
                        for i in range(bl) : data[x+i,y+frame_id*self.height]=color
                    elif self.datacode == DATA_cpl : 
                        color = self.palette[ord(self.f.read(1))]
                        if TYPECOLOR : color = ((0,0,255,255),(0,0,200,255))

                        for i in range(0,bl-1,2) : 
                            data[x+i  ,y+frame_id*self.height]=color[0]
                            data[x+i+1,y+frame_id*self.height]=color[1]
                        if bl%2 : 
                            data[x+bl-1,y+frame_id*self.height]=color[0]

                    else : 
                        raise ValueError, 'unknown datacode'

                elif bc == CODE_SKIP : 
                    pass

                elif bc == CODE_REF : 
                    # read back data index as u16
                    idx = struct.unpack('<H',self.f.read(2))
                    if self.datacode == DATA_cpl : 
                        pixels = [self.palette[0][0]]*bl
                    else : 
                        pixels = [(255,0,255,255)]*bl
                    if TYPECOLOR : color = (0,255,0,255)  
                    for i,color in enumerate(pixels) : 
                        data[x+i,y+frame_id*self.height] = color


                if beol :   
                    y+=1
                    x=0
                    if DEBUG : self.img.save('debug.png')

                else : 
                    x+=bl

    def __str__(self) :
        s= "%dx%d, %d frames, datacode : %d"%(self.width, self.height, len(self.frames_index), self.datacode)
        s+= '\nhitbox : %d,%d-%d,%d'%self.hitbox
        if self.palette : s += "\n%d couples in palette"%len(self.palette)
        return s

if __name__=='__main__' : 
    s = Sprite(sys.argv[1]) 
    print s
    s.unpack()
    s.img.save(sys.argv[1]+'.png')

