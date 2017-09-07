#!/usr/bin/env python
"""make a 64x64 16c icon as C source for a bitbox title from a graphical file.
This will export a bitbox_icon that can be linked into your game.

file format of icons : 

	u32 header
	u16 colors[16]
	u4  pixel_data[64x64]
	char  message[4][32] 

seen as a u16[]
"""

from PIL import Image
import struct
import sys

DEBUG=False

img = Image.open(sys.argv[1]).convert('P',palette=Image.ADAPTIVE, colors=16).crop((0,0,64,64))

if DEBUG : img.save('_debug.png')

pixels = img.load()

def u16color(r,g,b) :
    'R8G8B8 to A1R5G5B5'
    return (1<<15 | (r>>3)<<10 | (g>>3)<<5 |b>>3)

print '// Bitbox Game icon C file.'
print '#include <stdint.h>'
print 'const uint16_t bitbox_icon[2+16+64*64/4+128] = {'
print '    // - Header'
print '    0xB17B,0x01C0,  '
pal_it = iter(img.getpalette()[:48]) # iterator over 16 rgb triples

# write palette
print '    // - Palette 16c'
print '   ',
for r,g,b in zip(pal_it,pal_it,pal_it) : 
	print "0x%x,"%u16color(r,g,b),
print
# write pixel data 4 by 4
print "    // - Pixel data : 4bpp x 64x64"
for y in range(64):
	print '   ',
	for x in range(0,64,4):
		print "0x%x%x%x%x,"%(pixels[x,y],pixels[x+1,y],pixels[x+2,y],pixels[x+3,y]),
	print

# write text 
comment = img.info.get('Comment','')+'\n\n\n\n' # ensure at least 4 lines
for line in comment.split('\n')[:4] :
	ln = (line+' '*(32-len(line)))[:32]
	i = iter(ln)
	print '    //',ln
	print '   ',
	for a,b in zip(i,i) : 
		print "0x%x,"%(ord(b)<<8|ord(a)),
	print 

print '};'
