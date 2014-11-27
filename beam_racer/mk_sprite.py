''' an utility to extract raw 16bpp data from a pixmap'''
from PIL import Image # PIL  is python-imaging package on debian/ubuntu. See http://en.wikipedia.org/wiki/Python_Imaging_Library
import sys

filename =  sys.argv[1]

src = Image.open(filename)
dim = src.size
src = src.convert("RGB") # Un-paletize

print "/* *** File : %s *** */ "%filename
print "#include <stdint.h>"
print "const uint16_t carsprite[%d][%d] = {\n"%(dim[1],dim[0]),

for y in range(dim[1]) :
	print "\n	{"
	for x in range(0, dim[0]) :
		pix = src.getpixel((x,y))
		print "0x%04x,"%((pix[0]>>3)<<10 | (pix[1]>>3)<<5 | (pix[2]>>3)), 
		if (x%16)==15 : print "\n   ",
	print "\n },"

print "\n };"
