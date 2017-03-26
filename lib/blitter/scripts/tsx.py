#!/usr/bin/python2
"""
    exports a spritesheet file from a tsx file
    header file is output on the standard output.

"""

import sys
import argparse
from argparse import Namespace
import xml.etree.ElementTree as ET
import os
from collections import defaultdict
import StringIO # for output to C file
import glob

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

import sprite_encode2
import sprite_encode8
import couples_encode2

parser = argparse.ArgumentParser(description='Process TMX files to tset/tmap/.h files')
parser.add_argument('file',help='input .tsx filename', nargs='+')
parser.add_argument('-o','--output-dir', help='target directory for objects data', default='')
parser.add_argument('-m','--micro', default=False, help='outputs a 8-bit data tileset from a palette.', action="store_true")

args = parser.parse_args()

PALETTE = os.path.dirname(__file__)+'/pal_micro.png'

def error(msg) :
    print msg
    sys.exit(1)

class Sprite : 
    """A sprite comes from a tileset (Only one per tileset)

    For each state, a list of animations + hitbox.
    it comes from a sprite-related tileset element.

    If a tsx sprite is found in several, it will be re-exported. However it will be the same tsx source file
    so it will generate the same spr file. 
    """

    def __init__(self,filename) : 

        self.filename = filename
        self.ts=ET.parse(filename).getroot()

        self.ts = self.ts
        
        self.name = self.ts.get('name')

        self.image = self.ts.find('image').get('source')
        self.ts_w = int(self.ts.get('tilewidth'))
        self.ts_h = int(self.ts.get('tileheight'))
        self.tilecount = int(self.ts.get('tilecount'))

        self.tiles = [] # tile_id  looal to tileset
        self.states = [] # state name, tid, list of indices in self.tiles, hitbox

        self.read_states()

    def state_bytid(self,tid) : 
        "return a state from its tid"
        return next(st for st in self.states if st.tid==tid)

    def read_states(self) : 
        """read sprite animations, states and list used tiles"""
        for tile in self.ts.findall('tile') : 
            tid = int(tile.get('id')) # local id

            # read properties of this tile
            props = { p.get('name'):p.get('value') for p in tile.findall('properties/property')}
           
            if 'state' not in props : continue # not a state

            # get animation tiles
            anim_elt = tile.find('animation')
            if anim_elt != None : # animated tile
                frames = [ int(frame_elt.get('tileid')) for frame_elt in anim_elt.findall('frame')] # XX also duration ?
            else : # no animation : animation is made from only one tile : the tile id itself
                frames = [ tid ]
            self.tiles += [ x for x in frames if x not in self.tiles ] # new ones are added at the end so that order is not changed

            # find hitbox or set it empty
            try : 
                hit_elt = tile.find('objectgroup').find('object') # take first hit object  <object id="1" x="10.75" y="15.125" width="10.375" height="11.375"/>
                hitbox=(
                    int(float(hit_elt.get('x'))),
                    int(float(hit_elt.get('y'))),
                    int(float(hit_elt.get('x'))+float(hit_elt.get('width'))),
                    int(float(hit_elt.get('y'))+float(hit_elt.get('height')))
                )
            except AttributeError,e : 
                hitbox=(0,0,0,0)

            # create the state with its frames as references in used_tiles + hitbox
            state = props['state']
            self.states.append(Namespace(tid=tid,state=state,frames=[self.tiles.index(i) for i in frames],hitbox=hitbox))


    def __str__(self) : 
        s = 'Sprite %s (image:%s)\n'%(self.name, self.image)
        s += '  Tiles %s\n'%self.tiles
        s += '  States\n'
        for st in self.states : 
            s += "   %s\n"%st
        return s

    def export_sprite(self, path) : 
        "export the tiles .spr image for all used tiles as a sprite"
        imgsrc=abspath(self.filename, self.image)
        tileset=Image.open(imgsrc).convert('RGBA')

        # build a picture list from tile list
        tile_per_line = (tileset.size[0]/self.ts_w)
        srcs = [tileset.crop((
            (tid % tile_per_line)*self.ts_w,
            (tid //tile_per_line)*self.ts_h,
            (tid % tile_per_line)*self.ts_w+self.ts_w,
            (tid //tile_per_line)*self.ts_h+self.ts_h
        )) for tid in self.tiles]

        # build vertical stripe from list of srcs tiles.
        src = Image.new("RGBA", (self.ts_w,self.ts_h*len(srcs)))
        for i,im in enumerate(srcs) :
            src.paste(im,(0,i*self.ts_h))

        # SAVE_SPRITES ?
        # src.save(outfile.name+'.png')  # make it an option ?

        # export data as spr XXX to pb8
        with open(os.path.join(path,self.name+'.spr'),'w+') as outfile :
            print "/* Sprite data : ",imgsrc,len(srcs),' frames, in file:',outfile.name
            if args.micro :
                sprite_encode8.image_encode(src,outfile,self.ts_h,'u8')
            else :
                sprite_encode2.image_encode(src,outfile,self.ts_h,'p4')
            print '*/'
    
    def print_header(self) : 
        'prints a header with all data from map'
        print 'enum states_%s {'%self.name
        for st in self.states : 
            print '    state_%s_%s,'%(self.name,st.state)
        print '\n    %s_nbstates'%self.name
        print '};'
        print 'extern const struct SpriteDef sprite_'+self.name+';'

    def print_implementation(self) : 
        print 'const struct SpriteDef sprite_%s= {'%self.name
        print '  .file= data_%s_spr,'%self.name
        print '  .states= (const struct StateDef []){'
        maxstatelen = max(len(st.state) for st in self.states)
        for st in self.states:  
            print  '    [state_%s_%s]={.nb_frames=%2d, .frames=(uint8_t []) {%s},.x1=%d,.y1=%d,.x2=%d,.y2=%d},'%( 
                self.name,
                st.state.ljust(maxstatelen),
                len(st.frames),
                ','.join(str(x) for x in st.frames),
                st.hitbox[0],
                st.hitbox[1],
                st.hitbox[2],
                st.hitbox[3],
                )
        print '  }'
        print '};\n'

def abspath(ref, path) : 
    "transform relative path from ref file to absolute"
    return os.path.join(os.path.dirname(os.path.abspath(ref)),path)


print '#include <stdint.h>'
print '#include "lib/blitter/mapdefs.h"'
print '#include "data.h" // for resource ids'
print

# + objgroup,type ; name,in mor + defines

# gathers all spritesheets / export once, by name+prefix

sprites = [
    Sprite(tsxfile)
    for tsxfile in args.file
]

for sp in sprites : 
    sp.print_header()
    print

print "\n#ifdef SPRITE_IMPLEMENTATION // "+'-'*50+'\n'

for sp in sprites : 
    sp.export_sprite(args.output_dir)
    sp.print_implementation()


print "#endif // SPRITE_IMPLEMENTATION"

#for map in maps : 
#    print map # export mor / full maps + object map
