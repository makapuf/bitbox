#!/usr/bin/python2
'''
Generates dependencies for all tmx files given in input
'''

import sys
import os
import xml.etree.ElementTree as ET

CPATH='.'
DATAPATH='data'

def relpath(ref, path) : 
    "transform relative path from ref file to absolute"
    return os.path.join(os.path.dirname(ref),path)

tmxfiles = sys.argv[1:]
tsxfiles = set() 
generated = set() 

print "# This is an automaticaly genetrated file for tmx files",' '.join(tmxfiles)


for tmxfile in tmxfiles : 
	root = ET.parse(tmxfile).getroot()

	# -- dependencies
	imgs=set()
	tsxs=set()
	for tset in root.findall('tileset') : 
		if not 'name' in tset.attrib : 
			tsxfile = relpath(tmxfile,tset.get('source'))
			tsxs.add(tsxfile)
		else : 
			img = tset.find('image').get('source')
			imgs.add(relpath(tmxfile,img))


	# -- productions
	basename=os.path.splitext(os.path.basename(tmxfile))[0]

	hfile=os.path.join(CPATH,'map_'+basename+'.h')
	tmapfile = os.path.join(DATAPATH,basename+'.tmap')
	tset_base =  os.path.splitext(os.path.basename(root.find('tileset').find('image').get("source")))[0]
	tsetfile = os.path.join(DATAPATH,tset_base+'.tset')


	# -- outputs rules
	print hfile, tmapfile, tsetfile,':',tmxfile,' '.join(imgs),' '.join(tsxs)
	print '\t$(TMX) "%s" > %s'%(tmxfile,hfile)
	print

	# -- keep track
	generated.update([tmapfile,tsetfile])
	tsxfiles |= tsxs


for tsxfile in tsxfiles :	
	tset=ET.parse(tsxfile).getroot()
	
	# -- dependencies 
	img=tset.find('image').get('source')

	# -- production
	basename = tset.get('name')

	hfile    = os.path.join(CPATH,'sprite_'+basename+'.h')
	sprfile  = os.path.join(DATAPATH,basename+'.spr')

	# -- rules
	print hfile,sprfile,':',tsxfile,relpath(tsxfile,img)
	print '\t $(TSX) "%s" > %s'%(tsxfile,hfile)

	print

	generated.add(sprfile)

# -- for global data embedding
print 'TMX_DATAFILES:=',' '.join(generated)
