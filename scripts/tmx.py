"""outputs a tileset file, a tilemap file binaries from a tmx file

    tileset file has the name of the input tmx tileset (only one).tset
    tilemap file has all layers in order of appearance, its name is tmxfile.tmap
    header file is output on the standard output, includes 
        tile names (property "name" of the tile) 
        objects layers
    
    also exports tile names 

    skip layers beginning with '_'

    only one image tileset will be encoded, however other tilesets can be used for items (objects, ...) - they must not be referenced in tilemap and should be used AFTER the main one.

"""


"""TODO : 
object tile_id relative to first tile of tilemap file (or even set a name for it and use it)

semi transp, signed integer
paths : // besoin de nom sinon crie
    2 lists i16 layername_pathname_x[],layername_pathname_y[]

better multilayer : array of ptrs with its own header?
use struct instead of array module

rewrite tileset ? at least, dont output last unused tiles 

"""

out_code='B' # unsigned bytes by default
typename = {'B':'uint8_t', 'b' : 'int8_t','H':'uint16_t'}
codes = {'B':'TMAP_U8', 'H':'TMAP_U16' }#0,'b':1,'H':2,'h':3}
tilesizes = {16:'TSET_16',32:'TSET_32', 8:'TSET_8'}

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
import sys, argparse
import xml.etree.ElementTree as ET
import array, os.path, argparse

parser = argparse.ArgumentParser(description='Process TMX files to tset/tmap/.h files')
parser.add_argument('file',help='input .tmx filename')
parser.add_argument('-o','--output-dir', default='.', help='target directory, default: .')
parser.add_argument('-c','--to_c_file', default=False, help='outputs directly a C file instead of binaries.', action="store_true")

args = parser.parse_args()

tree = ET.parse(args.file)
root = tree.getroot()
def error(msg) : 
    print msg
    sys.exit(1)

base_name = args.file.rsplit('.',1)[0]

tilesets = root.findall('tileset')

print '#include <stdint.h>'


# tileset properties : name is a unique property - export for all tilesets allowing item-only names 
# TODO : relative to tileset firstGID ? 
tilenames = set()
print '// tile ids'
for ts in tilesets : 
    for tile in ts.findall('tile') : 
        tid = int(tile.get('id'))+int(ts.get('firstgid')) 
        properties = {prop.get('name'):prop.get('value') for prop in tile.findall('./properties/property')}
        if 'name' in properties : 
            tilename = properties['name']
            if tilename in tilenames : 
                print "// warning duplicate name %s of tile id %d"%(tilename,tid)
            print "#define %s_%s %d"%(base_name,tilename,tid)       
            tilenames.add(properties['name'])


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

c_file = open(os.path.join(args.output_dir,base_name+'.c'),'wb')
print >>c_file,'#include <stdint.h>'
print >>c_file,'#include "%s.h"'%base_name

src = Image.open(os.path.join(os.path.dirname(os.path.abspath(args.file)),img)).convert('RGBA')
pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples. 
w,h = src.size
with open(os.path.join(args.output_dir,base_name+'.tset'),'wb') as of: 
    if args.to_c_file : 
        print >>c_file,"const uint16_t %s_tset[] = { // from %s"%(base_name,img)
    for tile_y in range(h/tilesize) : 
        for tile_x in range(w/tilesize) : 
            for row in range(tilesize) : 
                idx = (tile_y*tilesize+row)*w + tile_x*tilesize
                if args.to_c_file : 
                    print >>c_file, ",".join(str(x) for x in pixdata[idx:idx+tilesize]),','
                else : 
                    pixdata[idx:idx+tilesize].tofile(of) # non extraire des carres de 16x16 avec PIL et les ecrire 
if args.to_c_file : 
    print >>c_file,"};"


index=0
if not args.to_c_file : 
    of = open(os.path.join(args.output_dir,base_name+'.tmap'),'wb') 

mw, mh = root.get('width'), root.get('height')
if args.to_c_file : 
    print >>c_file, "const %s %s_tmap[][%s*%s]={"%(typename[out_code], base_name,mw,mh)
    
print '// layers'    
for layer in root.findall("layer") : 
    lw = int(layer.get("width"))
    lh = int(layer.get("height"))
    name=layer.get("name")
    if name[0]=='_' : continue # skip
    
    data = layer.find("data")
    indices = [int(s) for s in data.text.replace("\n",'').split(',')]
    if max(indices)>=256 : out_code='H' # will set to u16 if just one level has those indices
    tidx = array.array(out_code,indices)
    assert len(tidx) == lw*lh, "not enough or too much data"
    
    print "#define %s_%s %d"%(base_name, name, index)
    # output data to binary 
    if args.to_c_file : 
        print >>c_file, "{ // %s "%name
        print >>c_file, ",".join(str(x) for x in tidx)
        print >>c_file, "},"
    else : 
        tidx.tofile(of)
    index += 1

# output all layers in a big array of arrays
if args.to_c_file : 
    print >>c_file, "};"

print '// max indices : ',max(indices)

# output object layers to C file
for objectgroup in root.findall('objectgroup') : 
    name=objectgroup.get('name')
    if name[0]=='_' : continue # skip

    pos = [(int(float(o.get('x'))),int(float(o.get('y'))), int(o.get('gid'))) for o in objectgroup.findall('object')]

    print "#define %s_%s_nb %d"%(base_name,name,len(pos))
    print "extern const int16_t %s_%s[%s_%s_nb][4]; // x,y,tid,0"%(base_name,name,base_name,name)

    print >> c_file,"const int16_t %s_%s[%s_%s_nb][4] = { // x,y,tid,0 "%(base_name,name,base_name,name)
    for c in pos :
        print >>c_file, "    {%d,%d,%d,0},"%c
    print >>c_file, "};\n"

print "#define %s_header TMAP_HEADER(%d,%d,%s,%s)"%(base_name,lw,lh,tilesizes[tilesize],codes[out_code])

print "extern const uint16_t %s_tset[]; // from %s"%(base_name,img)
print "extern const %s %s_tmap[][%d*%d];"%(typename[out_code], base_name,lw,lh)
