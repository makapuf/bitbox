"""outputs a tileset file, a tilemap file binaries from a tmx file

    tileset file has the name of the input tmx tileset (only one).tset
    tilemap file has all layers in order of appearance, its name is tmxfile.tmap
    header file is output on the standard output
    also exports tile names 
"""


"""TODO : semi transp, signed integer

accepter des tilesets commencant par _ (ne pas exporter)

export layer objects ! 
    tiles ou autres objets : 1 liste
    filename_layername[3][] = {{x,y,type},{x,y,type}, ...}
        type = id type, a definir comme types, enum filename_object_XXX, filename_object_YYY

    paths : // besoin de nom sinon crie
        2 lists i16 layername_pathname_x[],layername_pathname_y[]

better multilayer : array of ptrs with its own header?
use struct instead of array ? 

"""

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
import sys, argparse
import xml.etree.ElementTree as ET
import array, os.path

tree = ET.parse(sys.argv[1])
root = tree.getroot()
def error(msg) : 
    print msg
    sys.exit(1)

base_name = sys.argv[1].rsplit('.',1)[0]

tilesets = root.findall('tileset')
ts = tilesets[0]
# checks
firstgid=int(ts.get('firstgid'))
assert len(tilesets)==1 and firstgid==1 , "Just one tileset starting at 1"

tilesize = int(ts.get("tilewidth"))

assert tilesize == int(ts.get("tileheight")) and tilesize in [16,32], "only square tiles of 32x32 or 16x16 supported"

# tileset properties : name is a unique property
tilenames = set()
print '// tile ids'
for tile in ts.findall('tile') : 
    tid = int(tile.get('id'))+firstgid 
    properties = {prop.get('name'):prop.get('value') for prop in tile.findall('./properties/property')}
    tilename = properties['name']
    if tilename in tilenames : 
        print "// warning duplicate name %s of tile id %d"%(tilename,tid)
    print "#define %s_%s %d"%(base_name,tilename,tid)       
    tilenames.add(properties['name'])


img = ts.find("image").get("source")
print "extern const uint16_t %s_tset[]; // from %s"%(base_name,img)
# output as raw tileset 
# XXX if RGBA -> int, else : uint. Here, assume uint
def reduce(c) : 
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 


src = Image.open(os.path.join(os.path.dirname(sys.argv[1]),img)).convert('RGBA')
pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples. 
w,h = src.size
tsname = base_name+'.tset'
with open(tsname,'wb') as of: 
    for tile_y in range(h/tilesize) : 
        for tile_x in range(w/tilesize) : 
            for row in range(tilesize) : 
                idx = (tile_y*tilesize+row)*w + tile_x*tilesize
                pixdata[idx:idx+tilesize].tofile(of) # non extraire des carres de 16x16 avec PIL et les ecrire 


out_code='B' # bytes
typename = {'B':'uint8_t', 'b' : 'int8_t','H':'uint16_t'}
codes = {'B':'TMAP_U8', 'H':'TMAP_U16' }#0,'b':1,'H':2,'h':3}
tilesizes = {16:0, 32:1}

fname = base_name+'.tmap'
index=0
with open(fname,'wb') as of : 
    for layer in root.findall("layer") : 
        lw = int(layer.get("width"))
        lh = int(layer.get("height"))
        name=layer.get("name")
        
        data = layer.find("data")
        indices = [int(s) for s in data.text.replace("\n",'').split(',')]
        if max(indices)>=256 : out_code='H' # will set to u16 if just one level has those indices
        tidx = array.array(out_code,indices)
        assert len(tidx) == lw*lh, "not enough or too much data"
        
        # output data
        print "const int %s_%s = %d;"%(base_name, name, index)
        tidx.tofile(of)
        index += 1

    print '// max indices : ',max(indices)
    # output object layers
    for objectgroup in root.findall('objectgroup') : 
        name=objectgroup.get('name')
        pos = [(int(o.get('x')),int(o.get('y')), int(o.get('gid'))) for o in objectgroup.findall('object')]

        print "#define %s_%s_nb %d"%(base_name,name,len(pos))
        print "const uint16_t %s_%s[%s_%s_nb][4] = {"%(base_name,name,base_name,name)
        for c in pos :
            print "    {%d,%d,%d,0},"%c
        print "};"

print "const uint32_t %s_header = TMAP_HEADER(%d,%d,%d,%s);"%(base_name,lw,lh,tilesizes[tilesize],codes[out_code])
print "extern const %s %s_tmap[][%d*%d];"%(typename[out_code], base_name,lw,lh)
