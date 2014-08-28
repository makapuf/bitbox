# exporte les donnees du jeu comme source C
from PIL import Image
print "const unsigned char *oiseau_data[2]= {"
for name in 'crappybird0','crappybird1' : 
	it = Image.open(name+'.png').getdata()
	its = [iter(it)] * 4 # group 4 by 4, voir itertools doc, chercher "grouper"
	print '(const unsigned char *)&(const unsigned char[]){'+repr(','.join("0x%02x"%(d<<6 | c<<4 | b<<2 | a) for a,b,c,d in zip(*its)))[1:-2]+'},'
print "};"

'''
import wave
for wavname in ('test',) : 
	w = wave.open(wavname+'.wav')
	assert w.getsampwidth()==1 and w.getnchannels()==1,'Wrong format, must be mono u8 4khz'
	print 'char * %s_wav="%r"' % (wavname,w.readframes(w.getnframes()))
'''