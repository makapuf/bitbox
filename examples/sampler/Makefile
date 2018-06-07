NAME = test_sampler
BITBOX = ../../sdk
# 80x25 text
DEFINES += VGAMODE=640 FONT_W=8 FONT_H=16

GAME_C_FILES = demo.c snd/song.c  $(BITBOX)/lib/sampler/sampler.c $(BITBOX)/lib/textmode/textmode.c 
GAME_BINARY_FILES = $(wildcard snd/*.raw)

include $(BITBOX)/kernel/bitbox.mk

demo.c: data.h

data.h : $(GAME_BINARY_FILES)
	$(BITBOX)/lib/resources/embed.py --prefix=snd_ $^ > $@


.DELETE_ON_ERROR: snd/song.c
snd/song.c: snd/satie_gnossienne1.mid
	python $(BITBOX)/lib/sampler/sampler_read_midi.py $^ > $@

clean:: 
	rm -f snd/song.c
