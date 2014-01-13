import sys, itertools

for i in range(4) : sys.stdin.readline()

d=[]
for l in sys.stdin :
	try :  
		t=l.strip().split()
		d.append((int(t[0],16),t[5],int(t[4],16)))

	except IndexError,ValueError : 
		pass

for a,b,c in sorted(d,key=lambda x:x[1]) : print b,c


n=0
for a,b,c in sorted(d,key=lambda x:x[2],reverse=True) : 
	n += c
	print b,c,n
print '---'
print sum(x[2] for x in d)

def grouper(x) : 
	if '_' not in x[1] : return 0
	return x[1].split('_')[0]

gr = [(n,sum(a[2] for a in u)) for n,u in itertools.groupby(sorted(d,key=grouper),key=grouper)]
n=0
for a,b in sorted(gr,key=lambda x:x[1], reverse=True) : 
	n += b
	print a,b,n