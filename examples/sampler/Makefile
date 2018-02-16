NAME = test_sampler
BITBOX = ../../sdk
# 80x25 text
DEFINES += VGAMODE=640 FONT_W=8 FONT_H=16

GAME_C_FILES = demo.c snd/song.c lib/sampler/sampler.c lib/textmode/textmode.c 
GAME_BINARY_FILES = $(wildcard snd/*.raw)

# bitbox_simple.mk, will include files by default
include $(BITBOX)/kernel/bitbox.mk

demo.c: build/binaries.h

.DELETE_ON_ERROR: snd/song.c
snd/song.c: snd/satie_gnossienne1.mid
	python $(BITBOX)/lib/sampler/sampler_read_midi.py $^ > $@

clean:: 
	rm -f snd/song.c
