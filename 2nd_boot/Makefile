NAME = 2nd_boot
GAME_C_FILES = $(NAME).c font.c bg.c  
DEFINES = VGAMODE_640
USE_SDCARD = 1
NO_AUDIO=1

# will only be used when not in emulator
KERNEL_BITBOX = flashit.c

# comment to build to flash. This will load boot2 as a normal program in flash 
# BUT obviously cannot program flash anymore
LINKER_RAM=1

include $(BITBOX)/lib/bitbox.mk

#.INTERMEDIATE: font.c
font.c: 8x16_437.png
	python mk_font.py $< > $@

bg.c: bg.png
	python mk_bg.py $< > $@

