''' an utility to transform pixmaps of 8x16 and 6x8 fonts to C arrays of uint8_t '''

from PIL import Image
import sys

def font(filename, size_x, size_y) : 
	print "/* *** File : %s *** */ "%filename
	print "const uint8_t %s_data[256][%d] = {"%(filename,size_y)
	src = Image.open(filename+'.png')

	for y in range(src.size[1]//size_y) : 
	    for x in range(src.size[0]//size_x) : 
	        block = tuple(src.crop((x*size_x,y*size_y,(x+1)*size_x,(y+1)*size_y)).getdata())
	        bytes = ["0x%02x"%int("".join("01"[i] for i in block[l*size_x:(l+1)*size_x]),2) for l in range(size_y)]
	        print "{",", ".join(bytes),"}, //",y*16+x,repr(chr(y*16+x))
	print "};"

print "#include <stdint.h>"
font('font16',8,16)
font('font8',6,8)
font('font88',8,8)
