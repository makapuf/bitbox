''' an utility to transform pixmaps of 8x16 fonts to C arrays of uint8_t '''

from PIL import Image
import sys

filename = sys.argv[1] # '8x16_437.png'

print "/* *** File : %s *** */ "%filename
print "#include <stdint.h>"
print "uint8_t font_data[256][16] = {"
src = Image.open(filename)

for y in range(src.size[1]//16) : 
    for x in range(src.size[0]//8) : 
        block = tuple(src.crop((x*8,y*16,(x+1)*8,(y+1)*16)).getdata())
        bytes = ["0x%02x"%int("".join("01"[i] for i in block[l*8:l*8+8]),2) for l in range(16)]
        print "{",", ".join(bytes),"}, //",y*16+x,repr(chr(y*16+x))
print "};"