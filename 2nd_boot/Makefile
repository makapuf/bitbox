NAME = 2nd_boot
GAME_C_FILES = $(NAME).c font.c bg.c flashit.c default_icon.c
GAME_BINARY_FILES = 
DEFINES = VGA_MODE=640 PAL_MODE=672
USE_SDCARD = 1
NO_AUDIO=1

# comment to build to flash. This will load boot2 as a normal program in flash 
# BUT obviously cannot program flash anymore
LINKER_RAM=1
$(warning $(BITBOX))

include $(BITBOX)/kernel/bitbox.mk

#.INTERMEDIATE: font.c
font.c: 8x16_437.png
	python mk_font.py $< > $@

bg.c: bg.png
	python mk_bg.py $< > $@

default_icon.c: icon.png
	$(BITBOX)/kernel/mk_ico.py $^ > $@

clean:: 
	rm default_icon.c