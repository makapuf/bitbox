#testkernel makefile
NAME = testsdio
GAME_C_FILES = main.c 
EXTRA_FILES = image.bin

include ../bitbox.mk
image.bin : hardcore.png
	mkdir -p "build/img"
	python encode_image.py hardcore.png

anim.bin : 
	mkdir -p "build/img"
	mogrify -path "build/img"  -ordered-dither o8x8,32 +dither -colors 255 "../../../../bitbox/logo/render/????.png"
	python encode_anim.py "build/img/????.png"
