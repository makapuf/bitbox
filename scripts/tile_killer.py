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
args = parser.parse_args()
print 'reducing tiles (tilesize %d,'%args.tilesize+('hflip' if args.hflip else '')+('vflip' if args.vflip else '')+')'

TS=args.tilesize
src = Image.open(args.file)
w,h = src.size

seen_tiles = set()
same=diff=empty=0
for tile_y in range(h/TS) : 
    for tile_x in range(w/TS) : 
        im = src.crop((tile_x*TS,tile_y*TS,(tile_x+1)*TS,(tile_y+1)*TS))
        data = tuple(im.getdata())
        if all(x[3]==0 for x in data) : 
            empty+=1
        else : 
            if data in seen_tiles :
                im.putdata([(255,0,255)]*TS*TS) # fill the tile
                # put it back 
                src.paste(im,(tile_x*TS,tile_y*TS))
                same+=1
            else: 
                seen_tiles.add(data)
                if args.hflip :
                    seen_tiles.add(tuple(im.transpose(Image.FLIP_LEFT_RIGHT).getdata()))
                if args.vflip :
                    seen_tiles.add(tuple(im.transpose(Image.FLIP_TOP_BOTTOM).getdata()))
                diff+=1

src.save(args.file.replace('.png','_unique.png'))
print empty,'empty',same,'same,',diff,'different tiles', TS*TS*diff,'B.'
print "tilesize",h/TS,'x',w/TS,(same+diff)*2,'B'

