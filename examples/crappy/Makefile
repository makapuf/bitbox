#testkernel makefile
NO_AUDIO=1

# set kernel interface to 8 bits. 

DEFINES = VGA_MODE=400 VGA_BPP=8 

NAME = crappy
GAME_C_FILES = crappy.c icon.c
include $(BITBOX)/kernel/bitbox.mk

icon.c: icon.png
	python $(BITBOX)/lib/blitter/scripts/mk_ico.py $^ > $@

clean::
	rm -f icon.c