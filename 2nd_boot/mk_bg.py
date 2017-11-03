''' an utility to transform a pixmap to a 256 color bg palette'''
from PIL import Image # PIL  is python-imaging package on debian/ubuntu. See http://en.wikipedia.org/wiki/Python_Imaging_Library
import sys

filename =  sys.argv[1] # '8x16_437.png'

print "/* *** File : %s *** */ "%filename
print "#include <stdint.h>"
print "const uint16_t bg_data[256] = {\n   ",
src = Image.open(filename)

for y in range(256) : 
	pix = src.getpixel((0,y))
	print "0x%04x,"%((pix[0]>>3)<<10 | (pix[1]>>3)<<5 | (pix[2]>>3)), 
	if (y%16)==15 : print "\n   ",
print "\n };"