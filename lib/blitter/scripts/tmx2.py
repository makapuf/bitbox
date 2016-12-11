#!/usr/bin/python2
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
DOC
export a,b,c custom object data (or even change name)

export as a unique .h

sprites :
    hidden groups / layers positions are not exported
    hitbox (as 4bytes ?)
    options to set number of colors of sprites ?
    multilayer ?
    external tilesheets for shared sprites between tmx ?
tilemap :
    export properties from terrains (handle blocking as 4 bits ?)

    handle H?V symmetry ?
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
import sprite_encode2, sprite_encode8, couples_encode2
import StringIO # for output to C file

parser = argparse.ArgumentParser(description='Process TMX files to tset/tmap/.h files')
parser.add_argument('file',help='input .tmx filename')
parser.add_argument('-o','--output-dir', help='target directory, default: tmx file source')
parser.add_argument('-m','--micro', default=False, help='outputs a 8-bit data tileset from a palette.', action="store_true")
parser.add_argument('-a','--export-tile-attributes', default=False, help='exports a bitfield property from tiles from is_xxx boolean tile attributes as an u8 bin array, after tileset data.',action="store_true")
parser.add_argument('-x','--export-objects', default=False, help='exports object data directly to c file.', action="store_true")
parser.add_argument('-s','--export-sprites', default=False, help='exports object data in c file and sprites as 16c spr files. Honors micro palette.',action="store_true")
parser.add_argument('-i','--export-images', default=False, help='exports images layer as spr pbc files. Honors micro palette.',action="store_true")

#parser.add_argument('-g','--group-anims', default=False, help='exports animations groups - by object type. animations will be a u8** instead of u8',action="store_true")

args = parser.parse_args()

PALETTE = os.path.dirname(__file__)+'/pal_micro.png'

tree = ET.parse(args.file)
root = tree.getroot()
def error(msg) :
    print msg
    sys.exit(1)

base_path,base_name = os.path.split(args.file.rsplit('.',1)[0])
if args.output_dir :
    base_path = args.output_dir
dirname=base_path.replace('/','_')+'_' if base_path else ''


print '#include <stdint.h>'


# tileset properties : name is a unique property - export for all tilesets allowing item-only names
# TODO : relative to tileset firstGID ? rewrite skipping ununsed tid.
tilesets = root.findall('tileset')
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

def ts_from_gid(gid) :
    # find first tileset that has a greater TID, takes the one before.
    for ts in root.findall('tileset') :
        if int(ts.get('firstgid'))>gid:
            break
        else :
            last_ts=ts
    return last_ts

def export_sprite(outfile,tiles,tileset_elt) :
    "exports and encode a sprite info outfile from a list of gid in the tileset"

    ts_w = int(tileset_elt.get('tilewidth'))
    ts_h = int(tileset_elt.get('tileheight'))
    firstgid = int(tileset_elt.get('firstgid'))

    imgsrc = tileset_elt.find('image').get('source')

    tileset=Image.open(os.path.join(os.path.dirname(os.path.abspath(args.file)),imgsrc)).convert('RGBA')

    # build a picture list from tile list
    tiles_local = [tid-firstgid for tid in tiles]
    tile_per_line = (tileset.size[0]/ts_w)
    srcs = [tileset.crop((
        (tid % tile_per_line)*ts_w,
        (tid //tile_per_line)*ts_h,
        (tid % tile_per_line)*ts_w+ts_w,
        (tid //tile_per_line)*ts_h+ts_h
    )) for tid in tiles_local]

    # build vertical stripe from list of srcs tiles.
    src = Image.new("RGBA", (ts_w,ts_h*len(srcs)))
    for i,im in enumerate(srcs) :
        src.paste(im,(0,i*ts_h))

    # SAVE_SPRITES ?
    # src.save(outfile.name+'.png')  # make it an option ?

    # export data as spr XXX to pb8
    print "/* Sprite data : ",imgsrc,len(srcs),' frames, in file:',sprfile
    if args.micro :
        sprite_encode8.image_encode(src,outfile,ts_h,'u8')
    else :
        sprite_encode2.image_encode(src,outfile,ts_h,'p4')
    print '*/'


# export first tileset - IIF there is a tilemap defined
# XXX get all used tiles from tilemaps and make a tileset, remap 

if root.findall('layer') : 

    ts = tilesets[0]
    # checks
    firstgid=int(ts.get('firstgid'))

    # assert len(tilesets)==1 and firstgid==1 , "Just one tileset starting at 1"

    tilesize = int(ts.get("tilewidth"))
    assert tilesize == int(root.get("tileheight")) and tilesize in tilesizes, "only square tiles of 8,16 or 32 supported"


    img = ts.find("image").get("source")
    # output as raw tileset
    # XXX if RGBA -> int, else : uint. Here, assume uint
    def reduce(c) :
        return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0

    src = Image.open(os.path.join(os.path.dirname(os.path.abspath(args.file)),img)).convert('RGBA')
    if args.micro :
        src = src.convert('RGB').quantize(palette=Image.open(PALETTE)) # XXX allow 8-bit adaptive palette ?
        pixdata = array.array('B',src.getdata()) # direct output bytes
    else:
        pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples.

    print "// extern const uint%d_t %s%s_tset[]; // from %s"%(8 if args.micro else 16,dirname, base_name,img)
    w,h = src.size

    with open(os.path.join(base_path,base_name+'.tset'),'wb') as of:
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
                prop = sum(1<<n for n,k in enumerate(props) if tid in tilebools[k])
                of.write(chr(prop))
            print "#define %s_tset_attrs_offset %s"%(base_name, pos_attrs-1),'// offset in bytes in tset file of tiles attibutes.'


print '\n // -- Tilemaps'
index=0
of = open(os.path.join(base_path,base_name+'.tmap'),'wb')
mw, mh = root.get('width'), root.get('height')

print '\n// Layers'
max_idx = 0
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
    max_idx = max(max_idx,max(indices))

# if there is a layer, export the global header 
if root.findall("layer") : 
    if args.micro :
        print "#define %s_header (TMAP_HEADER(%d,%d,%s,%s) | TSET_8bit)"%(base_name,lw,lh,tilesizes[tilesize],codes[out_code])
    else :
        print "#define %s_header TMAP_HEADER(%d,%d,%s,%s)"%(base_name,lw,lh,tilesizes[tilesize],codes[out_code])

    print "// extern const %s %s%s_tmap[][%d*%d];"%(typename[out_code],dirname, base_name,lw,lh)

# output all layers in a big array of arrays
print '// max indices : ',max_idx

# output object layers to C file.
if args.export_objects or args.export_sprites :
    print '\n// -- Objects '

    print 'struct MapObjectRef {\n    int16_t x,y;\n    uint8_t state_id; // or sprite_id\n    uint8_t a,b,c; // extra values  \n};'
    all_types = set() # typename : list of names(states for this type)
    all_states = {} # type, name index --> gid

    all_sprites = [] # tile_id of all sprites if export sprite

    c_file = StringIO.StringIO()

    for objectgroup in root.findall('objectgroup') :
        name=objectgroup.get('name')
        if name[0]=='_' : continue # skip

        # XXX also finds a,b,c bytes / only output shown in a shown layer
        pos = [(int(float(o.get('x'))),int(float(o.get('y')))-int(float(o.get('height'))), int(o.get('gid')), o.get('name',''), o.get('type','')) for o in objectgroup.findall('object')]
        for x,y,gid,nm,typ in pos :
            all_types.add(typ)

            if (typ,nm) in all_states  :
                if all_states[(typ,nm)]!=gid :
                    print >>sys.stdout,"Error : state %s_%s has multiple sprite definitions (tile_ids)."%(typ,nm)
                    sys.exit(1)
            else :
                all_states[(typ,nm)]=gid

        print "\n#define %s_%s_nb %d"%(base_name,name,len(pos))
        print "extern const struct MapObjectRef %s_%s[%s_%s_nb];"%(base_name,name,base_name,name)

        print >>c_file,"const struct MapObjectRef %s_%s[%s_%s_nb] = { // x,y,state_id "%(base_name,name,base_name,name)
        for x,y,gid,nm,typ in pos :
            print >>c_file, "    {%d,%d,%s,0,0,0},"%(x,y,'%s_st_%s_%s'%(base_name, typ,nm))
        print >>c_file, "};\n"


    all_states_sorted = sorted(all_states)
    all_types_sorted = sorted(all_types)

    # export global names and types
    print "// types and states"
    print "enum %s_type {"%base_name
    print ',\n'.join('    %s_t_%s'%(base_name,nm) for nm in all_types_sorted)
    print '};'
    print '#define %s_t_nb %d'%(base_name, len(all_types_sorted))

    current_type = None
    print '\nenum %s_states {'%base_name
    print ', \n'.join('    %s_st_%s_%s'%(base_name, t,n) for t,n in all_states_sorted)
    print '};'

    print "\n#define %s_st_nb %d"%(base_name,len(all_states_sorted))
    print "extern const uint8_t %s_types[%s_st_nb]; // lookup table state -> type"%(base_name,base_name)
    print >>c_file,"const uint8_t %s_types[%s_st_nb] = {  "%(base_name,base_name)
    for t,n in all_states_sorted :
        print >>c_file, "    %d, // %s"%(all_types_sorted.index(t), '%s_%s'%(t,n))
    print >>c_file, "};\n"

    # export sprites themselves and reference them in a table
    if args.export_sprites :
        print "\n// sprites"
        print "extern void *%s_sprites[%s_t_nb]; // pointers to spr files, one per type (non-const since they are ints to be loaded dynamically)"%(base_name,base_name)
        print "extern uint8_t *%s_anims[%s_st_nb]; // pointers to array of u8 animation data, one per state"%(base_name,base_name)
        print "extern uint8_t %s_hitbox[%s_st_nb][4]; // hitbox for a state (x1,y1,x2,y2) "%(base_name,base_name)

        print

        print >>c_file, "\n// sprites (see refs from data_h)"
        print >>c_file, "void * %s_sprites[%s_t_nb] = { // sprites[type_id] -> sprite data (ref, will be ptr)"%(base_name,base_name)

        # Build a list of unique GID for each type
        typ_tiles = defaultdict(set) # dict typ : set of animation tiles
        typ_tileset = {} # typ:ts as xml element
        state_anims = [] # list of tid animations per state
        state_hitbox = [] # list of hitbox per state

        prev_type=None
        for (t,s) in all_states_sorted :
            tid = all_states[(t,s)]
            ts = ts_from_gid(tid)
            firstgid = int(ts.get('firstgid')) # first GID of this tileset
            typ_tileset[t]=ts

            # find animation gids. if no animation, animation is made from only one tile : the tile id
            animtile = ts.find("tile[@id='%s']"%(tid-firstgid)) # id within tileset

            anim_elt = animtile.find('animation') if animtile != None else None
            if anim_elt != None :
                animation_tid = [ int(frame_elt.get('tileid'))+firstgid for frame_elt in anim_elt.findall('frame')]
            else :
                animation_tid = [ tid ]

            # append to current types' set of unique tile GID
            typ_tiles[t] |= set(animation_tid)
            state_anims.append(animation_tid) # keep all animations

            # find hitbox or set it empty
            try : 
                hit = animtile.find('objectgroup').find('object') # take first one  <object id="1" x="10.75" y="15.125" width="10.375" height="11.375"/>
                state_hitbox.append((
                    int(float(hit.get('x'))),
                    int(float(hit.get('y'))),
                    int(float(hit.get('x'))+float(hit.get('width'))),
                    int(float(hit.get('y'))+float(hit.get('height')))
                ))
            except AttributeError,e : 
                state_hitbox.append((0,0,0,0))

        # freeze tile anims
        typ_tiles_sorted = { k:sorted(v) for k,v in typ_tiles.items() }

        # export animations as id of tiles for each sprite
        # export sprites themselves
        for t in sorted(typ_tiles_sorted) :
            # export data as spr
            sprfile='%s_%s.spr'%(base_name,t)
            with open(os.path.join(base_path,sprfile),'w+') as f :
                export_sprite(f,typ_tiles_sorted[t],typ_tileset[t])

            # list name as ext file
            sprname = dirname+sprfile.replace('.','_')
            print >>c_file,"   (void*)%s, "%sprname
        print >>c_file,'};'

        # animation data as remapped indexes in type tiles
        print >>c_file,"uint8_t *%s_anims[] = {"%base_name
        for (t,s),anim in zip(all_states_sorted, state_anims) :
            print >>c_file,'    (uint8_t []){'+','.join(str(typ_tiles_sorted[t].index(tid)) for tid in anim)+', 255 }, // %s_%s'%(t,s)
        print >>c_file,'};'

        # animations hitboxes
        print >>c_file,'uint8_t %s_hitbox[][4] = {'%base_name
        for (t,s),box in zip(all_states_sorted, state_hitbox):
            print >>c_file, '    {%d,%d,%d,%d}, // %s_%s'%(box[0],box[1],box[2],box[3],t,s)
        print >>c_file,'};'


    # export images as spr files in pbc format
    if args.export_images : 
        print "\n// Image Layers, exported as big pbc sprites"
        for imagelayer in root.findall('imagelayer') :
            imgname=imagelayer.find('image').get('source')
            src = Image.open(imgname).convert('RGBA')
            sprfile=base_name+'_'+imagelayer.get('name')+'.spr'
            with open(os.path.join(base_path,sprfile),'w+') as f :
                print '/* Image : ',imgname,'to file:',sprfile
                couples_encode2.couples_encode(src,f,src.size[1],'pbc',args.micro)
                print '*/'


    print "// "+'-'*80
    print "#ifdef TILEMAPS_IMPLEMENTATION"
    print c_file.getvalue()
    print "#endif"
