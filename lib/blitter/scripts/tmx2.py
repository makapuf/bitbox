#!/usr/bin/python2
"""outputs a tileset file, a tilemap file binaries and objects from a tmx file
    header file is output on the standard output, includes
        tile names (property "name" of the tile)
        objects layers
    TMX layer data can be either csv or zlib+base64 encodings

    skip layers beginning with '_'

    if data to export are more involved, define your own special script to export special attributes

    only the first tileset is output
    for sprites, internal sprite tilesets will not be output, only use external tilesets

TODO : terrains, tile names, zones (=sprites without a spr file ? )
"""

import sys
import argparse
from argparse import Namespace
import xml.etree.ElementTree as ET
import os.path
from collections import defaultdict
import StringIO # for output to C file
import array

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

import couples_encode2

parser = argparse.ArgumentParser(description='Process TMX files to tset/tmap/.h files')
parser.add_argument('file',help='input .tmx filename', nargs='+')
parser.add_argument('-o','--output-dir', help='target directory', default='.')
parser.add_argument('-m','--micro', default=False, help='outputs a 8-bit data tileset from a palette.', action="store_true")
parser.add_argument('-p','--prefix_files', help='Prefix for files',default='data_')

#parser.add_argument('-g','--group-anims', default=False, help='exports animations groups - by object type. animations will be a u8** instead of u8',action="store_true")

args = parser.parse_args()

PALETTE = os.path.dirname(__file__)+'/pal_micro.png'

def error(msg) :
    print >>sys.stderr,msg
    sys.exit(1)

class Sprite : 
    """A sprite comes from a tileset (Only one per tileset)

    For each state, a list of animations and a .
    it comes from a sprite-related tileset element.

    If a tsx sprite is found in several, it will be re-exported. However it will be the same tsx source file
    so it will generate the same spr file. 
    """

    def __init__(self,tileset_elt,prefix, ref) : 

        self.ts = tileset_elt
        self.ref = ref # reference of file from which source images path are taken

        # prefix to generate names. none for global tsx or tmx file name if local 
        self.name = prefix+tileset_elt.get('name')
        self.tilecount = int(tileset_elt.get('tilecount'))
        self.states = [] # state name, tid, list of indices in self.tiles, hitbox
        self.Layers=[] # has any layers
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
            if 'state' in props : 
                self.states.append(Namespace(tid=tid,state= props['state']))


    def __str__(self) : 
        return 'Sprite %s (states:%s)\n'%(self.name, self.states)

class Map :
    def __init__(self, tmxfile, dir=None) : 
        self.file = tmxfile
        self.name = os.path.basename(tmxfile.rsplit('.',1)[0])

        self.root = ET.parse(tmxfile).getroot()

        self.objects = [] 
        self.spritesheets = {} # actually loaded / used as sprites tilesets. first_gid : tileset/spritesheet

        self.load_spritesheets()
        self.load_objects()
        self.load_map()

    def load_spritesheets(self) : 
        "unconditionnally load all tilesets as spritesheets, discarding if no states found"
        for ts_elt in self.root.findall('tileset') : 
            if 'source' in ts_elt.attrib : # indirect one in tsx file
                tsx_file = abspath (self.file, ts_elt.get('source'))
                tsx= ET.parse(tsx_file).getroot()
                sprite = Sprite(tsx,'',tsx_file) # no prefix, it's global
            else  : 
                # this is a local tilemap
                sprite = Sprite(ts_elt,self.name+'_',self.file)

            if sprite.states : # discard no-state spritesheets ?
                self.spritesheets[int(ts_elt.get('firstgid'))] = sprite
            else : 
                print "// Tilemap %s was not included since it has no states defined"%sprite.name

    def load_objects(self) :
        for objectgroup_elt in self.root.findall('objectgroup') :
            groupname=objectgroup_elt.get('name')
            if groupname[0]=='_' : continue # skip
            
            # only get GID ones ie tile objects
            for o_elt in objectgroup_elt.findall('object[@gid]') : 
                obj = Namespace (
                    x=int(float(o_elt.get('x'))),
                    y=int(float(o_elt.get('y')))-int(float(o_elt.get('height'))),
                    gid=int(o_elt.get('gid')),

                    name=o_elt.get('name',None), # object name 
                    type=o_elt.get('type',None), # type
                    group=groupname,  # group
                    )

                # find tilesheet
                obj.sprite=self.find_spritesheet(obj.gid)
                obj.tid=obj.gid-obj.sprite

                # check that the gid of this object one of a state
                if obj.tid not in set(st.tid for st in self.spritesheets[obj.sprite].states) : 
                    error("Tile ID of object %s is not a state of sprite sheet %s"%(obj,self.spritesheets[obj.sprite]))
                
                self.objects.append(obj)

            # now group by spritesheet
            self.objects.sort(key=lambda x:x.tid)

    def load_map(self) : 
        if not self.root.find('tileset') : return 

        self.tilebools={}
        self.layers = self.root.findall("layer")
        tileset_src = self.root.find('tileset').find('image').get("source") 
        self.tileset= os.path.splitext(os.path.basename(tileset_src))[0]

    # export first tileset as _the_ tileset
    # FIXME define from external (or internal) tileset (reuse)
    # FIXME export tile properties (name at least)
    def export_tileset(self,output_dir) :
        ts = self.root.find('tileset')

        # export terrain types 
        print "// terrains"
        for n,t in enumerate(ts.findall('terraintypes/terrain')) : 
            print "#define terrain_%s_%s %d"%(self.name, t.get('name'),n+1)
            if n>15 : 
                print "ERROR : must have less than 16 terrains"
                sys.exit(1)

        tilesize = int(ts.get("tilewidth"))
        assert tilesize == int(self.root.get("tileheight")) and tilesize in [8,16,32], "only square tiles of 8,16 or 32 supported"

        img = ts.find("image").get("source")
        # output as raw tileset
        # XXX if RGBA -> int, else : uint. Here, assume uint
        def reduce(c) :
            return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0

        src = Image.open(abspath(self.file,img)).convert('RGBA')
        if args.micro :
            src = src.convert('RGB').quantize(palette=Image.open(PALETTE)) # FIXME allow 8-bit adaptive palette ? custom ?
            pixdata = array.array('B',src.getdata()) # direct output bytes
        else:
            pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples.

        w,h = src.size

        with open(os.path.join(output_dir,self.tileset+'.tset'),'wb') as of:
            for tile_y in range(h/tilesize) :
                for tile_x in range(w/tilesize) :
                    for row in range(tilesize) :
                        idx = (tile_y*tilesize+row)*w + tile_x*tilesize
                        pixdata[idx:idx+tilesize].tofile(of)
            
            # export terrains as 4 4bit numbers per tile
            terrains = array.array('H',(0 for i in range(int(ts.get('tilecount')))))
            for t in ts.findall('tile') : 
                ter = [(int(x)+1 if x else 0) for x in t.get('terrain',',,,').split(',')]
                terrains[int(t.get('id'))] = ter[0]<<12 | ter[1]<<8 | ter[2]<<4 | ter[3]
        self.terrains=terrains,w/tilesize

    def export_tilemap(self,out_dir) :
        index=0
        of = open(os.path.join(out_dir,self.name+'.tmap'),'wb')
        mw, mh = self.root.get('width'), self.root.get('height')

        print '\n// Tilemap layers'
        max_idx = 0
        for layer in self.layers :
            lw = int(layer.get("width"))
            lh = int(layer.get("height"))
            name=layer.get("name")
            if name[0]=='_' : continue # skip

            data = layer.find("data")
            if data.get('encoding')=='csv' :
                indices = [int(s) for s in data.text.replace("\n",'').split(',')]
            elif data.get('encoding')=='base64' and data.get('compression')=='zlib' :
                indices = array.array('I',data.text.decode('base64').decode('zlib'))
            else :
                raise ValueError,'Unsupported layer encoding :'+data.get('encoding')

            out_code='H' if max(indices)>=256 else 'B' # will set to u16 if just one level has those indices

            tidx = array.array(out_code,indices)
            assert len(tidx) == lw*lh, "not enough or too much data"

            print "#define layer_%s_%s %d"%(self.name, name, index)
            # output data to binary
            tidx.tofile(of)
            index += 1
            max_idx = max(max_idx,max(indices))

    def find_spritesheet(self, gid) : 
        "returns firstgid of spritesheet tileset in which the given gid is located"              
        for firstgid,ts in self.spritesheets.items() : 
            if firstgid<=gid and gid<firstgid+ts.tilecount : 
                return firstgid       
        raise ValueError,'gid %d not found in %d spritesheets'%(gid, len(self.spritesheets))

    def __str__(self) : 
        s = '-- map %s\n'%(self.name)
        s += ' - Objects \n'
        for o in self.objects : 
            s += '  %s\n'%o
        s += ' - Sprites \n'
        for spi,sp in self.spritesheets.items() : 
            s += '  %s : %s\n'%(spi,sp)
        return s
    
    def print_header(self) : 
        print 'extern const struct MapDef map_%s;'%self.name
        for n,t in enumerate(sorted(set(o.type for o in self.objects))) : 
            print '#define type_%s_%s %d'%(self.name,t,n)

        print

    def print_implementation(self) : 
        print 'const struct MapDef map_%s = {'%self.name
        if self.layers : 
            print '  .tileset=data_%s_tset,'%self.tileset
            print '  .tilemap_w=%d,'%int(self.root.get('width'))
            print '  .tilemap_h=%d,'%int(self.root.get('height'))
            print '  .tilesize=%d,'%int(self.root.get('tilewidth'))
            print '  .tilemap=data_%s_tmap,'%self.name

            print '  .terrains=(uint16_t[]){\n   ',
            terr,tw = self.terrains
            for i,t in enumerate(terr): 
                print "0x%04x,"%t,
                if i%tw==tw-1 : print "\n   ",
            print '  },'

        print '  .nb_objects=%d,'%len(self.objects)

        print '  .objects={'
        for obj in self.objects:  
            spr = self.spritesheets[obj.sprite]
            print '    { .x=%4d, .y=%4d, .sprite=&sprite_%s, .state_id=state_%s_%s, .type=type_%s_%s },'%(
                obj.x, obj.y,
                spr.name, 
                spr.name,spr.state_bytid(obj.tid).state,
                self.name,obj.type
                )
        print '  }'
        print '};\n'


    def export_image_layers(self, path) : 
        # export image layers as spr files in pbc format
        print "\n// Image Layers, exported as big pbc sprites"
        for imagelayer in self.root.findall('imagelayer') :
            imgname=imagelayer.find('image').get('source')
            src = Image.open(imgname).convert('RGBA')
            sprfile=self.name+'_'+imagelayer.get('name')+'.spr'
            with open(os.path.join(path,sprfile),'w+') as f :
                print '/* Image : ',imgname,'to file:',sprfile
                couples_encode2.couples_encode(src,f,src.size[1],'pbc',args.micro)
                print '*/'

def abspath(ref, path) : 
    "transform relative path from ref file to absolute"
    return os.path.join(os.path.dirname(os.path.abspath(ref)),path)

maps=[Map(f) for f in args.file]

print '#include <stdint.h>'
print '#include "lib/blitter/mapdefs.h"'
print '#include "data.h" // for resource ids'

print

# + objgroup,type ; name,in mor + defines

# include ts headers  
for map in maps : 
    for sprite in map.spritesheets.values(): 
        print '#include "sprite_%s.h"'%sprite.name

for map in maps: 
    if map.layers : 
        map.export_tileset(args.output_dir)
        map.export_tilemap(args.output_dir)
    print
    map.print_header()

    map.export_image_layers(args.output_dir)

print "#ifdef MAPS_IMPLEMENTATION // "+'-'*50+'\n'

for map in maps: 
    map.print_implementation()

print "\n#endif // MAPS_IMPLEMENTATION"
