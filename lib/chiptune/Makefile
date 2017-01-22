NAME=test_chip
SONG=test2

BITBOX?=../../

GAME_C_FILES=demo.c songs/$(SONG).c $(BITBOX)/lib/chiptune/chiptune.c $(BITBOX)/lib/chiptune/player.c
GAME_C_OPTS = -DSONG=$(SONG)_chipsong -DMAX_CHANNELS=4
NO_USB=1

include $(BITBOX)/kernel/bitbox.mk

songs/%.c: songs/%.song
	python $(BITBOX)/lib/chiptune/song2C.py $^ > $@

clean::
	rm -f songs/*.c
