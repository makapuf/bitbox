NAME = test_sampler

# 80x25 text
DEFINES += VGAMODE_640 FONT_W=8 FONT_H=16 

GAME_C_FILES = test_sampler.c song.c lib/sampler.c lib/textmode.c 
GAME_BINARY_FILES = $(wildcard snd/*.raw)

# bitbox_simple.mk, will include files by default
include $(BITBOX)/kernel/bitbox.mk

song.c: satie_gnossienne1.mid
	python $(BITBOX)/scripts/sampler_read_midi.py $< > $@
