NAME=test_chip
SONG=bomber

GAME_C_FILES=main.c $(SONG).c $(BITBOX)/lib/chiptune.c
GAME_C_OPTS = -DSONG=$(SONG)_chipsong
NO_USB=1

include $(BITBOX)/kernel/bitbox.mk

%.c: %.song
	python $(BITBOX)/scripts/song2C.py $^ > $@

clean::
	rm -f $(SONG).c
