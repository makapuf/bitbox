#!/usr/bin/python2

'''
Couples + packbits : allows blanks skipping, RLE encoding and fast sprite blits in general
(fewer palette fetch than per pixel palettes), well aligned.

Blits can be fill or copy
fill : either index0 = fully transparent or any other = not transparent.
copy : 1st pixel can be transparent OR last pixel, not any other in the blit.
=> repeating holes x.x.x. must be encoded with 3 blits (for now)

exemple : .......c.cdcdcdcdcdcdcd.....efghijkucdcdsd
couples :  .. .. .. .c .c    dc dc dc dc dc dc d. .. .. ef gh ij ku cd cd sd
runs    :  -------- ++ ++    ----------------- ++ ----- +++++++++++++++++++++
packbit :  3, 0     1,1 1,1  6,2               1,3 2,0  7,[4,5,6,7,8,8,9]
couples : .. .c dc d. ef gh ij ku cd sd
'''

'''
TODO : export depuis TMX avec ptr vers data direct, plus par fichier.
comme ca un seul type de sprite, rapide, transparency, tt bien.
plus tard, permettre blit a l'envers ? (depuis la fin et remonter)
'''

import sys,struct
from itertools import chain, groupby
from sprite_encode2 import add_record

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.
ALPHA_T=127 # alpha cutoff
MAX_BLIT=127 # 7bits len
MIN_RUN=4 # minimum run length

TRANSP8=239
TRANSP16=0 

# XXX better dist ! passer en YUV ? couples : de/favoriser coupure au couple
# passer command line avec multiple frames , generer contenus objet par defaut ? (ie ROM data, abcd aussi & copier direct ..
# reduce  first : ecrase 3 dernier bits (faire mieux dehors)
# ameliorer vector quantization

def avg(l) : return sum(l)/float(len(l)) if l else 0
def stddev(l,avg) : return sum(((x-avg)*(x-avg)) for x in l)/float(len(l)) if l else 0
def product(v) :
    x=1
    for k in v : x*=k
    return x

def sizecube(c) :
    "size of a cube along each dimension"
    return [max(x[d]for x in c)-min(x[d] for x in c) for d in [0,1,2,4,5,6]] if c else [0 for x in range(6)]

def quantize_couples(cpls,nb):
    # vector quantize couples
    uniq = set(cpls)
    if len(uniq)<=nb :
        print "directly use %d bins"%len(uniq)
        palette = list(uniq)
        invpal = dict((c,n) for n,c in enumerate(palette))
        return palette, invpal

    bins=[
            [ c for c  in cpls if c[3]< ALPHA_T and c[7]< ALPHA_T ],
            [ c for c  in cpls if c[3]>=ALPHA_T and c[7]< ALPHA_T ],
            [ c for c  in cpls if c[3]< ALPHA_T and c[7]>=ALPHA_T ],
            [ c for c  in cpls if c[3]>=ALPHA_T and c[7]>=ALPHA_T ],
        ]  # one per couple alpha possibilities, so they never get in the same bin (and 1bit cut alpha)

    print "  reducing from %d bins to %d"%(len(cpls),nb)

    stuck_bins=[] # bins which we wont subdivide - too small, ...

    while len(stuck_bins)+ len(bins)<nb :
        # find id of largest cube. largest = volume * nb of elements

        if len(bins)==0 : break

        if len(bins)%5==0 :
            cid = max(range(len(bins)),key=lambda i:len(bins[i])*product(sizecube(bins[i])))
        else :
            cid = max(range(len(bins)),key=lambda i:len(bins[i]))

        colorset =bins[cid]
        if sum(sizecube(bins[cid])) < 15 :
            stuck_bins.append(bins.pop(cid))
            continue


        # choix de l'axe : max d'ecart type
        avgs = [avg([c[i] for c in bins[cid]]) for i in range(8)]
        stdv = [stddev([c[i] for c in bins[cid]],avgs[i]) for i in range(8)]

        axis = max(range(len(stdv)), key=lambda x:stdv[x]) # never on alpha axis : in a bin, all alpha are the same

        # Cut at MEDIAN : remove bin & place two new, smaller, bins
        del bins[cid]

        bins.append([c for c in colorset if c[axis] >  avgs[axis] ])
        bins.append([c for c in colorset if c[axis] <= avgs[axis] ])

    bins += stuck_bins

    palette=[(0,0,0,0,0,0,0,0)] # start with transparent couple in position 0
    invpal = {}
    for cl in bins :
        # find best representant of cl : centroid ? most frequent ?
        #acl=tuple(int(round(avg([c[i] for c in cl])/32.)*31) for i in range(8))
        acl=tuple(int(avg([c[i] for c in cl]))&~7 for i in range(8))
        if any( x>255 for x in acl) : print acl, cl

        # cut alpha
        r1,g1,b1,a1,r2,g2,b2,a2 = acl
        acl = (r1,g1,b1, 255 if a1>ALPHA_T else 0, r2,g2,b2, 255 if a2>ALPHA_T else 0)

        if acl not in palette :
            palette.append(acl)

        # make an invert mapping (asserts not too big !) from mean
        for c in cl :
            invpal[c]=palette.index(acl) # takes first if already in palette
    return palette, invpal

def reduce_couple8(c) : 
    '2 x R8G8B8A8 to u16=2xR3G2B2L1 - micro palette'
    out=[]
    for n in 0,4 : # offset 0 and 4
        r,g,b =[(x*7+127)/255 for x in c[n:n+3]] # convert to three bits
        a=c[n+3]
        out.append( r<<5 | (g&~1)<<2 | (b&~1) | (g&1) if a>128 else TRANSP8 ) # OR g and b
    return out[1]<<8 | out[0]

def reduce_couple(c) :
    'R8G8B8A8 to A1R5G5B5 - x2 for couples'
    a = (1<<15 | (c[0]*31//255)<<10 | (c[1]*31//255)<<5 | c[2]*31//255) if c[3]>127 else TRANSP16
    b = (1<<15 | (c[4]*31//255)<<10 | (c[5]*31//255)<<5 | c[6]*31//255) if c[7]>127 else TRANSP16
    return b<<16|a # little endian

def packbits(l) :
    buf=[]
    t=iter(l)

    try :
        while(1) :
            runlen=0
            buf.append(next(t))
            if len(buf)>=MAX_BLIT :
                yield buf
                buf=[]
            if len(buf)>=MIN_RUN and all(c==buf[-1] for c in buf[-MIN_RUN:-1]):
                if buf[:-MIN_RUN]: yield buf[:-MIN_RUN]
                c=buf[-1]
                buf=[]
                runlen=MIN_RUN
                while 1 :
                    x=next(t)
                    if c==x and runlen<MAX_BLIT:
                        runlen+=1
                    else :
                        buf.append(x)
                        break
                yield runlen,c
    except StopIteration :
        if runlen : yield runlen,c
        if buf : yield buf


def couples_encode(img,f,frame_height, mode, micro, out_file=None):
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

    nbcol = 255 if w*h > 1024 else 64 # small images should not have a big couples palette -> XXX use less bits per couple ?
    pal,invpal = quantize_couples((couples), nbcol ) #
    intpal = [reduce_couple(p) for p in pal] # reduced palette
    intpal8 = [reduce_couple8(p) for p in pal] # idem but 8bit

    size = len(pal)*2
    s_blits=[] # all lines
    for y,line_couples in enumerate(all_line_couples) :
        # now encode line : (header + blit) x n
        if y%16==0 :
            ofs = sum(len(x) for x in s_blits)
            line16.append(ofs/2)

        blits = [] # a blit will be either a list of couples or a tuple (nb,couple)
        #print len(line_couples), [invpal[x] for x in line_couples]

        for bl in packbits(invpal[x] for x in line_couples) :
            if type(bl)==tuple :
                s_blits.append(struct.pack('BB',*bl))
            else :
                s_blits.append(struct.pack('%dB'%len(bl),*bl))

    # spr write

    add_record(f,'header',struct.pack("<2I",w,frame_height)) # 1 frame for now
    if micro : 
        add_record(f,'palette_couple8',struct.pack("<%dH"%len(intpal8),*intpal8))
    else : 
        add_record(f,'palette_couple',struct.pack("<%dL"%len(intpal),*intpal))
    add_record(f,mode,''.join(s_blits))
    add_record(f,'line16',struct.pack("%dH"%len(line16),*line16))
    add_record(f,'end','')

    """
    # no spr write
    f.write(struct.pack("<4H",w,frame_height,img.size[1]/frame_height,len(intpal))) # u32 w,frame_h,nbframes,nb pal
    f.write(struct.pack("<%dL"%len(intpal),*intpal)) # len pal x u32 couple
    f.write(struct.pack("H%dH"%len(line16),len(line16),*line16)) # len(line16), line16
    f.write(''.join(s_blits))
    """

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

    if out_file :
        img = Image.new("RGBA",(len(all_line_couples[0])*2,h))
        img.putdata(newdata)
        # out file
        img.info['transparency']=None # if so , FIXME handle alpha
        img.save(out_file)

if __name__=='__main__' :
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('file', help="Input file in order as frames.")
    parser.add_argument('-x','--frames_x',help="horizontal number of frames",type=int,default=1)
    parser.add_argument('-y','--frames_y',help="vertical number of frames",type=int,default=1)
    parser.add_argument('--micro', default=False, help="Use bitbox micro palette.", action='store_true')

    args = parser.parse_args()

    print '\n***',args

    # prepare input as vertically stacked images
    src = Image.open(args.file).convert('RGBA')

    dw=src.size[0]/args.frames_x
    dh=src.size[1]/args.frames_y
    dst = Image.new("RGBA", (dw,dh*args.frames_y*args.frames_x))

    for j in range(args.frames_y) :
        for i in range(args.frames_x) :
            im=src.crop((i*dw,j*dh,i*dw+dw,j*dh+dh))
            dst.paste(im,(0,dh*(j*args.frames_x+i)))

    # output
    f = open(args.file.rsplit('.',1)[0]+'.spr','wb+')
    couples_encode(dst,f,dh*args.frames_x*args.frames_y,'c8',args.micro, args.file+'.png')
    #
    #f.seek(0)
    #image_decode(f).save(args.file_out+'.png') #note that only the first is written (but all are decoded)


