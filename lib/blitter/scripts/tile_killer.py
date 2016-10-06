#!/usr/bin/python2
''' make tiles unique in tilemap
outputs out.png with removed duplicate tiles.
'''
from PIL import Image
# using the PIL library, maybe you'll need to install it. python should come with t.
import argparse, sys

parser = argparse.ArgumentParser(description='make tiles unique in a tileset.')
parser.add_argument('tilesize', type=int,help='size of each tile in pixels')
parser.add_argument('file', help='input file name(png)')
parser.add_argument('--hflip', help='can Horizontally flip tiles', action='store_true')
parser.add_argument('--vflip', help='can vertically flip tiles', action='store_true')
parser.add_argument('--tileheight',type=int, help='optionally, tile height (default = same as width)')
parser.add_argument('--pack', help='produces a packed image', action='store_true')

args = parser.parse_args()
if args.tileheight==None : args.tileheight=args.tilesize


TW=args.tilesize
TH=args.tileheight if args.tileheight else args.tilesize

src = Image.open(args.file).convert('RGBA')
w,h = src.size

tx = w/TW # nb tiles horizontally

print 'reducing tiles (tilesize %dx%d,'%(TW,TH)+('hflip' if args.hflip else '')+('vflip' if args.vflip else '')+')'

seen_tiles = set()
same=diff=empty=0
for tile_y in range(h/TH) :
    for tile_x in range(w/TW) :
        im = src.crop((tile_x*TW,tile_y*TH,(tile_x+1)*TW,(tile_y+1)*TH))
        data = tuple(im.getdata())
        if all(x[3]==0 for x in data) :
            empty+=1
        else :
            if data in seen_tiles :
                im.putdata([(255,0,255)]*TH*TW) # fill the tile
                src.paste(im,(tile_x*TW,tile_y*TH)) # put it back, filled
                same+=1
            else:
                seen_tiles.add(data)
                if args.pack :
                    # blit it on image at first place
                    src.paste(im,((diff%tx)*TW,(diff//tx)*TH))

                if args.hflip :
                    seen_tiles.add(tuple(im.transpose(Image.FLIP_LEFT_RIGHT).getdata()))
                if args.vflip :
                    seen_tiles.add(tuple(im.transpose(Image.FLIP_TOP_BOTTOM).getdata()))
                diff+=1

if args.pack :
    src=src.crop((0,0,w,(diff+tx-1)//tx*TH))

src.save(args.file.replace('.png','_unique.png'))

print "tileset     :",empty,'empty',same,'same,',diff,'different tiles', TW*TH*diff,'B.'
print "tilemap     :",w/TW,'x',h/TH,w//TW*(h//TH)*2,'B'
print "total (8b)  :", TW*TH*diff  +w//TW*(h//TH)*2
print "total (16b) :", TW*TH*diff*2+w//TW*(h//TH)*2
