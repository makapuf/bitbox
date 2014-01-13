#encoding: utf-8
'''sample blit algorithms'''

MAXINT = 32768
MININT = -32767
SCREEN_WIDTH = 640

class Blitter : 
	'a blitter is responsible for calling each object'
	'this is a non transparent blitter, ie no transparent is kept.'
	def __init__(self) : 
		# long standing variables
		self.objectlist = [] # sorted by y

		# frame-lived variables
		self.activelist = [] # currently blitting object, sorted by z => wat about opaque 
		self.current_line = 0 # current line to blit. (use a global ?)
		self.next_to_blit = 0

		# line-lived variables
		self.opaque = [] # list of x1,x2 opaque blits on the line. the list is non overlapping and has growing x
		self.transparent_fifo = [] # (x1,x2,b) fifo, filled top to bottom
		
	def insert_object(self,o) : 
		# insert sorted, should use insertion sort (only add during frame !)
		self.objectlist.append(o)

	def frame(self) : 
		# ensure sort by y (should be already sorted)
		self.objectlist.sort(key=lambda o:o.y)

		# rewind all drawable frames
		for o in self.objectlist : o.frame(0)
		self.activelist = [] 
		self.next_to_blit = 0 # next object to blit in objectlist
		self.current_line = 0

	def line(self) : 
		# drop past objects from active list
		for i,o in enumerate(self.activelist) : 
			if self.current_line > o.y+o.h : 
				del self.activelist[i] # remove from list

		# add new active objects
		while (self.next_to_blit<len(self.objectlist) and self.current_line>=self.objectlist[self.next_to_blit].y) :
			self.activelist.append(self.objectlist[self.next_to_blit])
			self.next_to_blit += 1
			self.activelist.sort(key=lambda o:o.z)

		# will gather opaque blits & order blits
		
		# reset opaque list to screen coordinates
		self.opaque = [(MININT,0), (SCREEN_WIDTH,MAXINT)] 

		for o in self.activelist : 
			o.line(self) # will call pre_draw and blit opaques

		while self.transparent_fifo : # in reality, can just scan in reverse.
			x1,x2,b = self.transparent_fifo.pop() 
			b.blit(x1,x2) 


	def pre_draw (self,x1,x2,b,is_opaque) : 
		'declare opaque for this line & call blitting'
		'args : x1,x2,is_opaque  : declares span. '
		'b : reference for callback to blit'
		# there will always be a blit after x2, since screen boundaries is an infinite blit
		# transparent = push a masked line to the draw FIFO, which will be emptied after the make opaque
		print "\npredraw %d-%d"%(x1,x2),"opaque" if is_opaque else "transp"
		# skip all past indices in the list. FIXME keep index in mem 
		bltindex = 0
		while (x1>=self.opaque[bltindex][1]) : bltindex +=1


		while True : 
			print self.opaque, 'idx:',bltindex,'range', x1, x2
			# are we starting within a blit or before ? 
			if (self.opaque[bltindex][0]<=x1) : 
				# skip to the blit end
				x1 = self.opaque[bltindex][1]
				# advance blit
				bltindex += 1

			# ok now we passed, we're sure to be after a blit and not inside it. 
			# we're also sure to have a preceding one

			# blitte jusqu'au prochain obstacle ou la fin 

			if x1>=x2 : break # already finished (includes end of line)

			# there is a next blit here since x1 would have been MAXINT if it was the last
			if x2<=self.opaque[bltindex][0] : # do we finish before or at the start of the next opaque 
				print 'we finish now'
				# blit all
				# blit start is end of preceding blit ?
				if is_opaque : 
					b.blit(x1,x2)
					# mark opaque
					if (x1==self.opaque[bltindex-1][1]) : 
						# extend the current blit
						self.opaque[bltindex-1]=(self.opaque[bltindex-1][0],x2)
					else : # create a new one up to end
						self.opaque.insert(bltindex,(x1,x2))
				else : 
					print 'pushed f',x1,x2
					self.transparent_fifo.append((x1,x2,b))
				break
			else : 
				if is_opaque : 
					# restrict & extend next one (we ARE in line with preceding here, so we extend)
					b.blit(x1,self.opaque[bltindex][0])
					# mark opaque
					self.opaque[bltindex]=(x1, self.opaque[bltindex][1])
				else : 
					print 'pushed',x1,  self.opaque[bltindex][0]
					self.transparent_fifo.append((x1,  self.opaque[bltindex][0], b))
				x1 = self.opaque[bltindex][1]
				bltindex += 1
		print 'now',self.opaque


class Drawable : 
	def __init__(self) : 
		self.x=self.y=self.z=0
		self.w=self.h=0

	def line(self,blitter) : 
		'calls make_opaque of the blitter (& blit it ?)'
		pass

	def blit(self, x1, x2) : 
		'''really blit it, transparently or not, for current line.
		Always called with growing x, without overlap. 
		e.g. : if we first blit from x1 to x2, then the next x'1,x'2 blit will be so that x'1>=x2 (also x2>x1)
		'''
		pass

	def frame(self, line) : 
		'''do whatever is needed to rewind for a new frame. 
		Also fast forwards to line "line", without blitting
		 - if we start blitting at line y by example.'''
		pass

class TestDrawable (Drawable) :
	"color square, aligned on axes"
	def __init__(self, color) : 
		Drawable.__init__(self)
		self.color = color

	def line(self, blitter) :
		blitter.pre_draw(self.x, self.x+self.w, self, True)

	def blit(self, x1, x2) : 
		if x1<x2 :
			print '** blit of %s from'%self.color,x1, 'to', x2 


class TransparentDrawable (Drawable) :
	"Transparent square aligned on axes"
	def __init__(self, color) : 
		Drawable.__init__(self)
		self.color = color

	def line(self, blitter) :
		blitter.pre_draw(self.x, self.x+self.w, self, False)

	def blit(self, x1, x2) : 
		if x1<x2:
			print '++ transparent blit of %s from'%self.color,x1, 'to', x2 

b=Blitter()

blits = [(0,10),(5,15),(25,30),(24,26),(12,32),(8,35),(0,640)] # x1,x2 sorted by low to high, same y
for z,(x1, x2) in enumerate(blits) : 
	o = TestDrawable('black')
	o.x = x1
	o.w = x2-x1
	o.z = 2*z
	o.h=10
	b.insert_object(o)

o=TransparentDrawable('grey')
o.x = -50 ; o.w = 400 ; o.z = 9 ; o.h = 10
b.insert_object(o)

o=TransparentDrawable('blue')
o.x = -50 ; o.w = 640+50 ; o.z = 11; o.h = 10
b.insert_object(o)


b.frame()
b.line()



