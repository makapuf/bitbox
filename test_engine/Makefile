#testkernel makefile
NAME = test_engine

GAME_C_FILES = main.c 
GAME_BINARY_FILES = img0001_p16.spr mechant3_16.spr bg.tset bg_bg.tmap
USE_ENGINE = 1

include $(BITBOX)/lib/bitbox.mk

bg.tset bg_bg.tmap : bg.tmx bg.png ../scripts/tmx.py
	python ../scripts/tmx.py bg.tmx

img0001_p16.spr : img0001_p16.png
	python ../scripts/sprite_encode1.py $< $@
