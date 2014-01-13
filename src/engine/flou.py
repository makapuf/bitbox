from PIL import Image

src = Image.open('shoot_base.png')
dst = Image.new("RGB",src.size)

for y in xrange(src.size[1]) : 
	for x in xrange(4,src.size[0]) : 
		try : 
			dst.putpixel((x,y),src.getpixel(((x-4) if (x+y)%2 else x,y)))
		except IndexError : 
			print x,y, src.size
			raise

dst.save('out.png')