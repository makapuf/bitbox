"""outputs a tileset file, a tilemap file binaries from a tmx file

    tileset file has the name of the input tmx tileset (only one).tset
    tilemap file has all layers in order of appearance, its name is tmxfile.tmap
    header file is output on the standard output, includes 
        tile names (property "name" of the tile) 
        objects layers
    TMX layer data can be either csv or zlib+base64 encodings
    also exports tile names 

    skip layers beginning with '_'

    only one image tileset will be encoded, however other tilesets can be used for items (objects, ...) 
      they must not be referenced in tilemap and should be used AFTER the main one.

    option export-objects : objects position are optionnally encoded MapObjectRef structs. exports raw tile_ids
    option export-sprites : export 16color sprites in spr files. replaces tile_ids with a reference to a sprite array. also supports tile animations.

    option export-tile-attributes : up to 8 binary attributes are added to tileset data. defined in tmx files by attaching is_xxx attributes to tiles (example : is_walkable, is ...).

    if data to export are more involved, define your own special script to export special attributes

"""


"""TODO : 
reorganize code with data objects : tileset objects, tmxobjects, tilemaps 


sprites : 
    options to set number of colors of sprites ? 
    honor micro palette.
    names/types are local to a group(layer) --> set global ?
    multilayer ? 
    external tilesheets for shared sprites between tmx ? 
tilemap : 
    allow change export order horizontally / vertically / bottom/up or top/down ex : xY yX ...  ?  
    allow reordering tiles / not including unused ones in tileset ? (optional, data could be organized by lines)
        at least, dont output last unused tiles 
        reorder tiles ? (unused, then rewrite map to check for similar tiles) => no, external tmx->tmx optimizer ?

    semi-transp, signed integers
    other stats ? 

tilesets : 
    allow multiple tilesets 
    

paths (avec un nom) : 1 i16 layername_pathname[][2]=x,y points ? 
"""

out_code='B' # unsigned bytes by default for tilemap
typename = {'B':'uint8_t', 'b':'int8_t','H':'uint16_t'}
codes = {'B':'TMAP_U8', 'H':'TMAP_U16' }#0,'b':1,'H':2,'h':3}
tilesizes = {16:'TSET_16',32:'TSET_32', 8:'TSET_8'}

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
import sys, argparse
import xml.etree.ElementTree as ET
import array, os.path, argparse
from collections import defaultdict
import sprite_encode2

# XXX
#sys.argv += ('-amxs','alterego/maps.tmx')
if len(sys.argv)==1 : sys.argv += ('-mxs','tmap.tmx')


parser = argparse.ArgumentParser(description='Process TMX files to tset/tmap/.h files')
parser.add_argument('file',help='input .tmx filename')
parser.add_argument('-o','--output-dir', default='.', help='target directory, default: .')
parser.add_argument('-m','--micro', default=False, help='outputs a 8-bit data tileset from a palette.', action="store_true")
parser.add_argument('-a','--export-tile-attributes', default=False, help='exports a bitfield property from tiles from is_xxx boolean tile attributes as an u8 bin array, after tileset data.',action="store_true")
parser.add_argument('-x','--export-objects', default=False, help='exports object data directly to c file.', action="store_true")
parser.add_argument('-s','--export-sprites', default=False, help='exports object data in c file and sprites as 16c spr files. Honors micro palette.',action="store_true")

#parser.add_argument('-g','--group-anims', default=False, help='exports animations groups - by object type. animations will be a u8** instead of u8',action="store_true")

args = parser.parse_args()

PALETTE = os.path.dirname(__file__)+'/pal_micro.png'

tree = ET.parse(args.file)
root = tree.getroot()
def error(msg) : 
    print msg
    sys.exit(1)

base_name = args.file.rsplit('.',1)[0].split('/')[-1] # no path.

tilesets = root.findall('tileset')

print '#include <stdint.h>'


# tileset properties : name is a unique property - export for all tilesets allowing item-only names 
# TODO : relative to tileset firstGID ? rewrite skipping ununsed tid.
tilenames = set()
tilebools = defaultdict(list) # property_name : list of tile_ids
print '\n// -- Tilesets '
print '// Tile ids'
for ts in tilesets : 
    for tile in ts.findall('tile') : 
        tid = int(tile.get('id'))+int(ts.get('firstgid')) 
        for prop in tile.findall('./properties/property') : 
            prop_name=prop.get('name')
            prop_value = prop.get('value') 

            if prop_name=='name' : 
                if prop_value in tilenames : 
                    print "// warning duplicate name %s of tile id %d"%(prop_value,tid)
                print "#define %s_%s %d"%(base_name,prop_value,tid)
                tilenames.add(prop_value)
            elif prop_name.startswith('is_') : 
                k=prop_name.split('is_')[1]
                tilebools[k].append(tid)



ts = tilesets[0]
# checks
firstgid=int(ts.get('firstgid'))

# assert len(tilesets)==1 and firstgid==1 , "Just one tileset starting at 1"

tilesize = int(ts.get("tilewidth"))

assert tilesize == int(ts.get("tileheight")) and tilesize in tilesizes, "only square tiles of 32x32 or 16x16 supported"

img = ts.find("image").get("source")
# output as raw tileset 
# XXX if RGBA -> int, else : uint. Here, assume uint
def reduce(c) : 
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

src = Image.open(os.path.join(os.path.dirname(os.path.abspath(args.file)),img)).convert('RGBA')
if args.micro : 
    src = src.convert('RGB').quantize(palette=Image.open(PALETTE)) # XXX allow 8-bit adaptive palette ?
    pixdata = array.array('B',src.getdata()) # direct output bytes
    print "extern const uint8_t %s_tset[]; // from %s"%(base_name,img)
else: 
    pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples. 
    print "extern const uint16_t %s_tset[]; // from %s"%(base_name,img)

w,h = src.size

with open(os.path.join(args.output_dir,base_name+'.tset'),'wb') as of: 
    for tile_y in range(h/tilesize) : 
        for tile_x in range(w/tilesize) : 
            for row in range(tilesize) : 
                idx = (tile_y*tilesize+row)*w + tile_x*tilesize
                pixdata[idx:idx+tilesize].tofile(of) 

    # tile properties output
    if args.export_tile_attributes and tilebools : 
        pos_attrs=of.tell()
        assert len(tilebools)<8, "max 8 properties per tmx file"
        props = sorted(tilebools) # keys
        max_tid = max(max(tiles) for tiles in tilebools.values()) # actually used ?

        # should be put as a _resource_
        print '\n// Tile properties'
        for n,p in enumerate(props):  
            print "#define %s_prop_%s %d"%(base_name,p,1<<n)

        for tid in range(1,w*h/tilesize/tilesize+1) : 
            of.write(chr(sum(1<<n for n,k in enumerate(props) if tid in tilebools[k])))
        print "#define %s_tset_attrs_offset %s"%(base_name, pos_attrs),'// offset in bytes in tset file of tiles attibutes.'
print '\n // -- Tilemaps'
index=0
of = open(os.path.join(args.output_dir,base_name+'.tmap'),'wb') 
mw, mh = root.get('width'), root.get('height')

print '\n// Layers'    
for layer in root.findall("layer") : 
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

    if max(indices)>=256 : out_code='H' # will set to u16 if just one level has those indices
    tidx = array.array(out_code,indices)
    assert len(tidx) == lw*lh, "not enough or too much data"
    
    print "#define %s_%s %d"%(base_name, name, index)
    # output data to binary 
    tidx.tofile(of)
    index += 1

print "#define %s_header TMAP_HEADER(%d,%d,%s,%s)"%(base_name,lw,lh,tilesizes[tilesize],codes[out_code])
print "extern const %s %s_tmap[][%d*%d];"%(typename[out_code], base_name,lw,lh)

# output all layers in a big array of arrays
print '// max indices : ',max(indices)

# output object layers to C file. 
if args.export_objects or args.export_sprites : 
    print '\n// -- Objects '    
    # XXX define  multi_anim here
    print 'struct MapObjectRef {\n    int16_t x,y;\n    uint16_t tile_id; // or sprite_id\n    uint8_t name, type; \n} __attribute__((packed));'
    all_names = ['']
    all_types = ['']
    all_sprites = [] # tile_id of all sprites if export sprite

    c_file = open(os.path.join(args.output_dir,base_name+'_objects.c'),'wb')
    print >>c_file,'#include "%s.h"'%base_name

    for objectgroup in root.findall('objectgroup') : 
        name=objectgroup.get('name')
        if name[0]=='_' : continue # skip

        pos = [(int(float(o.get('x'))),int(float(o.get('y'))), int(o.get('gid')), o.get('name',''), o.get('type','')) for o in objectgroup.findall('object')]
        for x in pos : 
            if x[3] not in all_names : all_names.append(x[3])
            if x[4] not in all_types : all_types.append(x[4])
            if args.export_sprites : 
                if x[2] not in all_sprites : all_sprites.append(x[2])

        print "\n#define %s_%s_nb %d"%(base_name,name,len(pos))
        print "extern const struct MapObjectRef %s_%s[%s_%s_nb];"%(base_name,name,base_name,name)

        print >> c_file,"const struct MapObjectRef %s_%s[%s_%s_nb] = { // x,y,tid,name_id,type_id "%(base_name,name,base_name,name)
        for c in pos :
            print >>c_file, "    {%d,%d,%d,%d,%d},"%(c[0],c[1],all_sprites.index(c[2]) if args.export_sprites else c[2],all_names.index(c[3]),all_types.index(c[4]))
        print >>c_file, "};\n"

        # export global names and types
        print "// names"
        for n,name in enumerate(all_names) : 
            print '#define %s_oname_%s %d'%(base_name,name or "NONE",n)
        print "// types"
        for n,typ in enumerate(all_types) : 
            print '#define %s_otype_%s %d'%(base_name,typ or "NONE",n)

        # export sprites themselves and reference them in a table
        if args.export_sprites : 
            print "\n// sprites"
            print "extern void *%s_sprites[]; // pointers to 16c spr files"%base_name
            print "extern uint8_t *%s_anims[]; // pointers to array of u8 animation data"%base_name

            print >>c_file, "void *%s_sprites[] = {"%base_name

            anims = [] # list of list of tile_ids

            for n,tid in enumerate(all_sprites) : 
                # get image to encode : a line of tiles, put vertically TODO export as animation. 

                # TODO optionally group by tile type / output multiple tiles.
                # find first tileset that has a greater TID, takes the one before.
                for ts in root.findall('tileset') : 
                    if int(ts.get('firstgid'))>tid:
                        break
                    else : 
                        last_ts=ts

                imgsrc = last_ts.find('image').get('source')
                tileset=Image.open(os.path.join(os.path.dirname(os.path.abspath(args.file)),imgsrc)).convert('RGBA')
                ts_w = int(last_ts.get('tilewidth'))
                ts_h = int(last_ts.get('tileheight'))
                firstgid = int(last_ts.get('firstgid'))

                animtile = last_ts.find("tile[@id='%s']"%(tid-firstgid))

                # find animation. if no animation, animation is done from the tile id
                anim_elt = animtile.find('animation')
                if anim_elt != None : 
                    animation_tid = [ int(frame_elt.get('tileid'))+firstgid for frame_elt in anim_elt.findall('frame')]
                else : 
                    animation_tid = [ tid + firstgid ]
                animation_tid_unique = sorted(set(animation_tid))
                animation_tid_remapped = [animation_tid_unique.index(frame) for frame in animation_tid]

                # print "***",tid, animation_tid_unique, animation_tid_remapped
                anims.append(animation_tid_remapped)
                
                # build a picture list from animation 
                tile_per_line = (tileset.size[0]/ts_w)
                tile_x = tid % tile_per_line
                tile_y = tid // tile_per_line
                
                srcs = [tileset.crop((
                    (tid % tile_per_line)*ts_w,
                    (tid //tile_per_line)*ts_h,
                    (tid % tile_per_line)*ts_w+ts_w,
                    (tid //tile_per_line)*ts_h+ts_h
                )) for tid in animation_tid_unique]

                # build vertical stripe from list of srcs tiles.
                src = Image.new("RGBA", (ts_w,ts_h*len(srcs)))
                for i,im in enumerate(srcs) : 
                    src.paste(im,(0,i*ts_h))
                
                # export data as spr
                #sprfile=imgsrc.rsplit('.',1)[0]+'.spr' type if grouped by name
                sprfile='%s_%d_%s.spr'%(base_name,tid,imgsrc.rsplit('.',1)[0])
                print "/* Sprite data : ",imgsrc,len(srcs),',frames,to:',sprfile

                f=open(sprfile,'w+')
                if args.micro : 
                    sprite_encode8.image_encode(src,f,ts_h) 
                else : 
                    sprite_encode2.image_encode(src,f,ts_h,'p4') 
                print "*/"

                # XXX encode for 8bit!
                # XXX encode anim list !

                print >>c_file,"    &%s, // tile %d, tile inside %d"%(sprfile.replace('.','_'),tid, tid-int(last_ts.get('firstgid')))
                # animation data

                #image_encode(src,f,frame_height,mode)
            print >>c_file,'};'

            print >>c_file,"uint8_t *%s_anims[] = {"%base_name
            for an in anims : 
                print >>c_file,'{'+','.join(str(tid) for tid in an)+', 255 },'
            print >>c_file,'};'



