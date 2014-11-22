#testkernel makefile
NO_SDCARD=1

NAME = beamracer
GAME_C_FILES = $(NAME).c roadtex.c carsprite.c
GAME_C_OPTS = -DVGAMODE_320
include $(BITBOX)/lib/bitbox.mk

roadtex.c: nolines.png
	python mk_road.py $< > $@

carsprite.c: car.png
	python mk_sprite.py $< > $@
