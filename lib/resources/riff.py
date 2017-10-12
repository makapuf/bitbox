"""
Riff file writer
"""
import struct 
def u32(n) : return struct.pack('<I',n)

def riff_list(name, elements) :
    s= "".join(riff_chunk(*e) for e in elements)
    return name+s

def riff_chunk(name,elt) :
    assert len(name)==4,repr(name)+" is not a valid name"
    if type(elt) == list : # XX make it enumerable ? a dict ? 
        elt = riff_list(name, elt)
        name='LIST'
    return name+u32((len(elt)))+elt+' '*(len(elt)%2)
    
def riff(name, *elts) : 
    l = riff_chunk(name,list(elts))
    return 'RIFF'+l[4:] # replace type 

def U32(s) : 
    return struct.unpack('<I',s)[0]

def riff_decode_list_body(s) : 
    l=[]
    while s :       
        dat,s = riff_decode_chunk(s)
        l.append(dat)
    return l

def riff_decode_chunk(s) : 
    head = s[:4]
    sz   = U32(s[4:8])
    dat  = s[8:8+sz]

    if head in ('RIFF','LIST') : 
        name = dat[:4]
        dat  = riff_decode_list_body(dat[4:])
    else : 
        name=head
    return (name,dat),s[8+sz+sz%2:]

def riff_decode(r) : 
    if r[:4]!='RIFF' : raise ValueError,"bad RIFF header"
    r,rest = riff_decode_chunk(r)
    return r

if __name__=='__main__' : 
    import sys
    from collections import OrderedDict

    r= riff("POUF",('plop',[('HEAD','123')]),('RIKF','ZBLOP'),("MUFA","koko"),('MUFM',"4"))
    print repr(r)
    print repr(riff_decode(r))

