#testkernel makefile
NAME = test_engine

GAME_C_FILES = main.c 
GAME_BINARY_FILES = ball.spr bg.tset bg.tmap

USE_ENGINE = 1
NO_SDCARD=1
NO_USB=1

include $(BITBOX)/lib/bitbox.mk
main.c:build/bg.h

bg.tset bg_bg.tmap build/bg.h: bg.tmx bg.png ../scripts/tmx.py
	mkdir build
	python ../scripts/tmx.py bg.tmx > build/bg.h

ball.spr : ball/????.png
	python ../scripts/sprite_encode2.py $@ $? -m p4
