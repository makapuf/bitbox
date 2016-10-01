# 80x25 text
VGA_SIMPLE_MODE=0 

NAME = test_sampler

GAME_C_FILES = test_sampler.c song.c lib/sampler.c lib/simple.c 
GAME_BINARY_FILES = $(wildcard snd/*.raw)

# bitbox_simple.mk, will include files by default
include $(BITBOX)/kernel/bitbox.mk

song.c: satie_gnossienne1.mid
	python $(BITBOX)/scripts/sampler_read_midi.py $< > $@
