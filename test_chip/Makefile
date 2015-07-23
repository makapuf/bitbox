NAME=test_chip
SONG=bomber

GAME_C_FILES=main.c $(SONG).c $(BITBOX)/lib/chiptune.c 
GAME_C_OPTS = -DSONG=$(SONG)_chipsong
NO_USB=1

#USE_CHIPTUNE=1 not now, will use internal replacement one

include $(BITBOX)/lib/bitbox.mk

%.c: %.song
	$(BITBOX)/scripts/song2C.py $^ > $@

clean::
	rm $(SONG).c
