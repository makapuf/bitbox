#testkernel makefile
NAME = test_blitter

GAME_C_FILES = main.c lib/blitter.c lib/blitter_tmap.c lib/blitter_sprites.c
DEFINES = VGA_MODE=640
GAME_BINARY_FILES = ball.spr bg.tset bg.tmap ball_small.spr

USE_ENGINE = 1
NO_USB=1


include $(BITBOX)/kernel/bitbox.mk
main.c:bg.h

bg.tset bg.tmap bg.h: bg.tmx bg.png 
	python2 ../scripts/tmx.py bg.tmx > bg.h

ball.spr : ball/????.png
	python2 ../scripts/couples_encode.py $@ $(sort $^)

ball_small.spr : ball_small/????.png
	python2 ../scripts/couples_encode.py $@ $(sort $^)


clean::
	rm -f *.spr *.spr.png
	rm -f bg.h