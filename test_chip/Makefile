NAME=test_chip
GAME_C_FILES=main.c bdash.c chiptune.c
NO_USB=1

#USE_CHIPTUNE=1 not now, will use internal replacement one

include $(BITBOX)/lib/bitbox.mk

bdash.c: bdash.song
	$(BITBOX)/scripts/song2C.py $^ > $@

