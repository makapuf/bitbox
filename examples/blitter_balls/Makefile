#testkernel makefile
NAME = test_blitter

BITBOX:=$(realpath ../../sdk/)

LIBS:= blitter blitter_tmap blitter_sprites3
GAME_C_FILES = main.c $(LIBS:%=$(BITBOX)/lib/blitter/%.c)

DEFINES = VGA_MODE=640 NB_small=40 NB_big=3

GAME_BINARY_FILES = bg.tset bg.map ball_small.spr

# add binary only if needed
ifeq (,$(findstring NB_big=0,$(DEFINES)))
GAME_BINARY_FILES += ball.spr
endif

NO_USB=1

include $(BITBOX)/kernel/bitbox.mk
main.c: bg.h

bg.tset: bg.png
	$(BITBOX)/lib/blitter/scripts/mk_tset.py -s16 $^

bg.map bg.h: bg.tmx bg.png # FIXME use mk_tmap mk_tset
	$(BITBOX)/lib/blitter/scripts/mk_tmap.py bg.tmx > bg.h

%.spr : %/????.png
	$(BITBOX)/lib/blitter/scripts/mk_spr.py -o $@ $(sort $^) -p COUPLES

clean_assets: clean
	rm -f bg.h bd_data.h
	rm -f *.spr *.spr.png
	rm -f *.map *.tset
