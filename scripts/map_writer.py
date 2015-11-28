'map builder : builds a map from a tilesize, a tileset and a pixmap'

from PIL import Image
# using the PIL library, maybe you'll need to install it. python should come with t.
import argparse, sys, os.path

parser = argparse.ArgumentParser(description='build a tilemap from an image and a tileset.')
parser.add_argument('tilesize', type=int,help='size of each tile in pixels')
parser.add_argument('file', help='input file name(png)')
parser.add_argument('tset', help='tileset file name(png)')
parser.add_argument('file_out',help='output file(.tmx)')
args = parser.parse_args()

TS=args.tilesize

# build the tileset (incl H and V tiles)
FLIPPED_HORIZONTALLY = 0x80000000
FLIPPED_VERTICALLY   = 0x40000000

src = Image.open(args.tset).convert('RGB')
w,h = src.size
if w%TS : print "beware, width not a multiple of tilesize"
if h%TS : print "beware, height not a multiple of tilesize"

tileset = {} # binary string : ID
for tile_y in range(h/TS) : 
    for tile_x in range(w/TS) : 
        im = src.crop((tile_x*TS,tile_y*TS,(tile_x+1)*TS,(tile_y+1)*TS))
        tile_id = tile_x + tile_y * (w/TS) + 1 

        tileset[ tuple(im.getdata()) ] = tile_id
        tileset[ tuple(im.transpose(Image.FLIP_LEFT_RIGHT).getdata()) ] = tile_id | FLIPPED_HORIZONTALLY
        tileset[ tuple(im.transpose(Image.FLIP_TOP_BOTTOM).getdata()) ] = tile_id | FLIPPED_VERTICALLY

# create tilemap
img = Image.open(args.file).convert('RGB')
w,h = img.size
tilemap = []
for tile_y in range(h/TS) : 
    for tile_x in range(w/TS) : 
        im = img.crop((tile_x*TS,tile_y*TS,(tile_x+1)*TS,(tile_y+1)*TS))
        tile_id = tileset.get(tuple(im.getdata()),0)

        tilemap.append( tile_id )

# generate TMX
with open(args.file_out,'w') as of : 
    of.write('<map version="1.0" orientation="orthogonal" width="%d" height="%d" tilewidth="%d" tileheight="%d">\n'%\
        (w/TS,h/TS,TS,TS))
    of.write ('<tileset firstgid="1" name="tilemap" tilewidth="%d" tileheight="%d" spacing="0" margin="0" >\n'%(TS, TS))
    of.write ('    <image source="%s"/>\n'%os.path.abspath(args.tset))
    of.write ('</tileset>\n')
    of.write ('<layer name="layer" width="%d" height="%d" >'%(w/TS,h/TS))
    of.write ('<data encoding="csv">')
    of.write (",".join("%d"%i for i in tilemap))
    of.write ('</data>\n</layer>\n</map>')
