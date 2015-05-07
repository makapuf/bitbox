# 80x25 text
VGA_SIMPLE_MODE=0 

USE_SAMPLER=1

NAME = test_sampler

GAME_C_FILES = test_sampler.c song.c
GAME_BINARY_FILES = $(wildcard snd/*.raw)

# bitbox_simple.mk, will include files by default
include $(BITBOX)/lib/bitbox.mk

song.c: satie_gnossienne1.mid
	python $(BITBOX)/scripts/read_midi.py $< > $@
