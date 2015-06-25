NAME=test_chip
SONG=rapid1

GAME_C_FILES=main.c $(SONG).c chiptune.c
GAME_C_OPTS = -DSONG=$(SONG)_chipsong
NO_USB=1

#USE_CHIPTUNE=1 not now, will use internal replacement one

include $(BITBOX)/lib/bitbox.mk

%.c: %.song
	$(BITBOX)/scripts/song2C.py $^ > $@

