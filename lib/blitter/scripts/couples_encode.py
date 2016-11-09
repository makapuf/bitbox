'''
this is intended to be richer than 16color data, better taking into account couple redundancy 
This should be faster than 16c blits
(however a 16color data CAN be made faster to blit with a RAM palette of 256 couples)

'''
DEBUG=False


import sys,struct
from itertools import chain, groupby
from sprite_encode2 import add_record

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
ALPHA_T=127

# XXX better dist ! passer en YUV ? couples : de/favoriser coupure au couple
# passer command line avec multiple frames , generer contenus objet par defaut ? (ie ROM data, abcd aussi & copier direct ..
# reduce  first : ecrase 3 dernier bits (faire mieux dehors)
# ameliorer vector quantization

from couples_encode2 import quantize_couples

def reduce_couple(c) :
    'R8G8B8A8 to A1R5G5B5 - x2 for couples'
    a = (1<<15 | (c[0]*31/255)<<10 | (c[1]*31/255)<<5 | c[2]*31/255) if c[3]>127 else 0 
    b = (1<<15 | (c[4]*31/255)<<10 | (c[5]*31/255)<<5 | c[6]*31/255) if c[7]>127 else 0 
    return b<<16|a # little endian


def couples_encode(img,f,frame_height, mode, out_file):
    w,h=img.size
    line16=[]

    # get all couples
    # premultiply alpha (could be faster with numpy)
    d=list((r,g,b,255) if a>ALPHA_T else (0,0,0,0) for (r,g,b,a) in img.getdata()) # cut alpha

    # ok now encode couples
    couples = []
    all_line_couples = []

    for y in range(h) : 
        line = d[y*w: y*w+w]
        if len(line)%2 : line.append(line[-1])
        line_couples = [line[i]+line[i+1] for i in range(0,len(line),2)]
        all_line_couples.append(line_couples)
        couples += line_couples

    print '  ',len(couples),"couples,",len(set(couples)),"differents"

    nbcol = 255 if w*h > 1024 else 64
    pal,invpal = quantize_couples((couples), nbcol ) #
    intpal = [reduce_couple(p) for p in pal] # reduced palette
    if DEBUG : 
        for n,p in enumerate(pal) : print n,p, "%08x"%reduce_couple(p)

    size = len(pal)*2
    s_blits=[] # all lines
    for y,line_couples in enumerate(all_line_couples) : 
        if y%16==0 : 
            ofs = sum(len(x) for x in s_blits)
            line16.append(ofs/4) # XXX use /4 but need to align 

        blits = []
        skipped=0
        for skip,g in groupby(line_couples, lambda x:x==(0,0,0,0,0,0,0,0)) : 
            t = tuple(g)
            if skip : 
                skipped = len(t)
                # if skip too big, split !
                while (skipped>127) : 
                    blits.append([127,(),False])
                    skipped -= 127
            else :
                # idem 
                while t :
                    blits.append([skipped,t[:127],False])
                    skipped=0
                    t=t[127:]
        
        # set EOL 
        if blits : 
            blits[-1][2]=True
        else : 
            blits.append([0,[],True])
            
        # now encode line : (header + blit) x n
        for skip, blit, eol in blits :      
            header=(skip<<9) | (len(blit) << 1) | (1 if eol else 0)
            sdata = [invpal[c] for c in blit]
            sdata+=(0,)* ((2-len(blit))%4) # pad to 2+4 (ie len%4==2 )
            if DEBUG : print 'skip',skip,'blt',len(blit),'eol',eol,sdata
            s_blits.append(struct.pack('<H%dB'%len(sdata), header,*sdata))
            


    add_record(f,'header',struct.pack("<2I",w,frame_height)) # 1 frame for now
    add_record(f,'palette_couple',struct.pack("<%dL"%len(intpal),*intpal))
    add_record(f,mode,''.join(s_blits))
    add_record(f,'line16',struct.pack("%dH"%len(line16),*line16))
    add_record(f,'end','')

    size=f.tell()
    print '// %d bytes (%d/1M), reduction by %.1f'%(size,1024*1024/size,2*float(w)*h/size)

    # outputting resulting image

    # project each component 
    tr1 = dict((k,tuple(pal[invpal[k]][:4])) for k in invpal)
    tr2 = dict((k,tuple(pal[invpal[k]][4:])) for k in invpal)

    # replace each couple by its transform
    newdata = []
    for l in all_line_couples : 
        newdata += list(chain(*[(tr1[c],tr2[c]) for c in l]))

    img = Image.new("RGBA",(len(all_line_couples[0])*2,h))
    img.putdata(newdata)
    # out file
    img.info['transparency']=None # if so , FIXME handle alpha
    img.save(out_file)

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("file_out",help="Output file name. Usually .spr")
parser.add_argument('file_in', nargs='+', help="Input files in order as frames.")
args = parser.parse_args()

print '\n***',args.file_in,'to',args.file_out,  args

# prepare input as vertically stacked images
srcs = [Image.open(filein).convert('RGBA') for filein in args.file_in]

Width,Height = srcs[0].size
if not all(im.size == (Width,Height) for im in srcs) : 
    print "All images must be the same size !"
    sys.exit(0)

src = Image.new("RGBA", (Width,Height*len(srcs)))
for i,im in enumerate(srcs) : 
    src.paste(im,(0,i*Height))
# output 
f = open(args.file_out,'wb+')
couples_encode(src,f,Height,'c8',args.file_out+'.png')
#
#f.seek(0)
#image_decode(f).save(args.file_out+'.png') #note that only the first is written (but all are decoded)


