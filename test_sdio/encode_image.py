'very simple encoder for one raw image data to .bin file. expects small data <32k pixels'

import sys, struct
from PIL import Image 

def reduce(c) :
    'R8G8B8A8 to A1R5G5B5' 
    return (1<<15 | (c[0]>>3)<<10 | (c[1]>>3)<<5 | c[2]>>3) if c[3]>127 else 0 

src = Image.open(sys.argv[1]).convert('RGBA')
print "dimensions : ",src.size
assert src.size[0]*src.size[1]<32000, "too big image ! (must fit in 64k)"
raw = [reduce(c) for c in src.getdata()]

outfile=open('image.bin','wb')
outfile.write(struct.pack('<H',0xb71b)) # B17B0x anagram :)
outfile.write(struct.pack('<2H',*src.size)) 
outfile.write(struct.pack('<%dH'%len(raw),*raw)) 
outfile.write(struct.pack('<H',0xb71b)) # end
