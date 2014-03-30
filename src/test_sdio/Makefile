#testkernel makefile
NAME = testsdio
GAME_C_FILES = main.c 
EXTRA_FILES = anim.bin

include ../bitbox.mk

anim.bin : 
	mkdir -p "build/img"
	mogrify -path "build/img"  -ordered-dither o8x8,32 +dither -colors 255 "../../../../bitbox/logo/render/????.png"
	python encode_logo.py "build/img/????.png"
