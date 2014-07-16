'one 64x64 or 32x32 tilemap, one tileset.'

"""TODO : other sizes, u8, semi transp, signed integer

zlib for tset

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

tilesets = root.findall('tileset')
ts = tilesets[0]
# checks
assert len(tilesets)==1 and ts.get('firstgid')=="1" , "Just one tileset starting at 1"
assert ts.get("tilewidth") == ts.get("tileheight") == "16", "wrong format, please use 16x16 tiles"

img = ts.find("image").get("source")
print "image : ",img
# output as raw tileset 
# XXX if RGBA -> int, else : uint. Here, assume uint
def reduce(c) : 
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

base_name = sys.argv[1].rsplit('.',1)[0]

src = Image.open(os.path.join(os.path.dirname(sys.argv[1]),img)).convert('RGBA')
pixdata = array.array('H',(reduce(c) for c in src.getdata())) # keep image in RAM as RGBA tuples. 
w,h = src.size
tsname = base_name+'.tset'
with open(tsname,'wb') as of: 
	for tile_y in range(h/16) : 
		for tile_x in range(w/16) : 
			for row in range(16) : 
				idx = (tile_y*16+row)*w + tile_x*16
				pixdata[idx:idx+16].tofile(of) # non extraire des carres de 16x16 !

for layer in root.findall("layer") : 
	lw = int(layer.get("width"))
	lh = int(layer.get("height"))
	assert lw==lh, "square data for now"
	assert lw in (32,64),"32 or 64 size plz"

	out_code='B'
	sz_code = {(64,64):0}[lw,lh]<< 2 

	header = [sz_code<<2,0,0,0] if out_code in ('B','b') else [sz_code<<2,0] 

	data = layer.find("data")
	tidx = array.array(out_code,header+[int(s) for s in data.text.replace("\n",'').split(',')])
	assert max(tidx)<256, "max 256 tiles plz"
	assert len(tidx)-len(header) == lw*lh, "not enough or too much data"
	
	# output data
	fname = base_name+'_'+layer.get("name")+'.tmap'
	print "%r %dx%d u8"%(fname, lw,lh)
	with open(fname,'wb') as of : 
		tidx.tofile(of)

