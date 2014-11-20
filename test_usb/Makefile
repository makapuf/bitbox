#NO_USB=1
NO_SDCARD=1

NAME = testusb
GAME_C_FILES = $(NAME).c font.c bg.c
GAME_C_OPTS = -DVGAMODE_640
include $(BITBOX)/lib/bitbox.mk

#.INTERMEDIATE: font.c
font.c: 8x16_437.png
	python mk_font.py $< > $@

bg.c: bg.png
	python mk_bg.py $< > $@
