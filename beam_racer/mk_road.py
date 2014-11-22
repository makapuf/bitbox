''' an utility to extract raw 16bpp data from a pixmap'''
from PIL import Image # PIL  is python-imaging package on debian/ubuntu. See http://en.wikipedia.org/wiki/Python_Imaging_Library
import sys

filename =  sys.argv[1]

src = Image.open(filename)
dim = src.size

print "/* *** File : %s *** */ "%filename
print "#include <stdint.h>"
print "const uint8_t roadtex[%d][%d] = {\n"%(dim[1],dim[0] / 2),

for y in range(dim[1]) :
	print "\n	{"
	for x in range(0, dim[0], 2) :
		pix1 = src.getpixel((x,y))
		pix2 = src.getpixel((x+1,y))
		print "0x%02x,"%((pix1 & 0x3) | ((pix2 & 0x3)<<2)), 
		if (x%16)==15 : print "\n   ",
	print "\n },"

print "\n };"
