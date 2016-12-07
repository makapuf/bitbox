NAME=test_chip
SONG=bomber

GAME_C_FILES=demo.c songs/$(SONG).c $(BITBOX)/lib/chiptune/chiptune.c
GAME_C_OPTS = -DSONG=$(SONG)_chipsong
NO_USB=1

include $(BITBOX)/kernel/bitbox.mk

songs/%.c: songs/%.song
	python $(BITBOX)/lib/chiptune/song2C.py $^ > $@

clean::
	rm -f songs/*.c
