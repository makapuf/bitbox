"""outputs a tileset file, a tilemap file binaries from a tmx file

	tileset file has the name of the input tmx tileset (only one).tset
	tilemap file has all layers in order of appearance, its name is tmxfile.tmap
	header file is output on the standard output
"""


"""TODO : other sizes, u8, semi transp, signed integer
use struct instead of array
export .h 

allows tset zlib encoding 
better multilayer : array of ptrs with its own header?

"""

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
import sys, argparse
import xml.etree.ElementTree as ET
import array, os.path

#sys.argv.append('../test_engine/bg.tmx')

tree = ET.parse(sys.argv[1])
root = tree.getroot()
def error(msg) : 
	print msg
	sys.exit(1)

base_name = sys.argv[1].rsplit('.',1)[0]

tilesets = root.findall('tileset')
ts = tilesets[0]
# checks
assert len(tilesets)==1 and ts.get('firstgid')=="1" , "Just one tileset starting at 1"
assert ts.get("tilewidth") == ts.get("tileheight") == "16", "wrong format, please use 16x16 tiles"

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
	for tile_y in range(h/16) : 
		for tile_x in range(w/16) : 
			for row in range(16) : 
				idx = (tile_y*16+row)*w + tile_x*16
				pixdata[idx:idx+16].tofile(of) # non extraire des carres de 16x16 avec PIL et les ecrire 

out_code='B' # bytes
typename = {'B':'uint8_t', 'b' : 'int8_t','H':'uint16_t'}
codes = {'B':1}#0,'b':1,'H':2,'h':3}

fname = base_name+'.tmap'
index=0
with open(fname,'wb') as of : 
	for layer in root.findall("layer") : 
		lw = int(layer.get("width"))
		lh = int(layer.get("height"))
		name=layer.get("name")
		

		data = layer.find("data")
		tidx = array.array(out_code,[int(s) for s in data.text.replace("\n",'').split(',')])
		assert max(tidx)<256, "max 256 tiles plz"
		assert len(tidx) == lw*lh, "not enough or too much data"
		
		# output data
		print "const int %s_%s = %d;"%(base_name, name, index)
		tidx.tofile(of)
		index += 1

print "const uint32_t %s_header = %d<<24 | %d<<16 | %d;"%(base_name, lw,lh,codes[out_code])
print "extern const %s %s_tmap[][%d*%d];"%(typename[out_code], base_name,lw,lh)
