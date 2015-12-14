#testkernel makefile
NAME = testsdio
GAME_C_FILES = main.c 
EXTRA_FILES = image.raw
USE_SDCARD=1
NO_USB=1

include $(BITBOX)/lib/bitbox.mk
image.raw : hardcore.png
	mkdir -p "build/img"
	python encode_image.py hardcore.png

