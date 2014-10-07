#testkernel makefile
NAME = testsdio
GAME_C_FILES = main.c 
EXTRA_FILES = image.bin

include $(BITBOX)/lib/bitbox.mk
image.bin : hardcore.png
	mkdir -p "build/img"
	python encode_image.py hardcore.png

