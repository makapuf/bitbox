#testkernel makefile
NAME = test_blitter

GAME_C_FILES = main.c lib/blitter/blitter.c lib/blitter/blitter_tmap.c lib/blitter/blitter_sprites3.c
DEFINES = VGA_MODE=640 
GAME_BINARY_FILES = ball.spr bg.tset bg.tmap ball_small.spr

NO_USB=1


include $(BITBOX)/kernel/bitbox.mk
main.c: bg.h

bg.tset bg.tmap bg.h: bg.tmx bg.png 
	python2 $(BITBOX)/lib/blitter/scripts/tmx.py bg.tmx > bg.h

%.spr : %/????.png
	python2 $(BITBOX)/lib/blitter/scripts/mk_spr.py -o $@ $(sort $^) -p COUPLES

clean::
	rm -f *.spr *.spr.png
	rm -f *.tmap *.tset 
	rm -f bg.h bd_data.h