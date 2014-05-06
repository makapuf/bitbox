
NAME = 2nd_boot
GAME_C_FILES = $(NAME).c font.c
include ../bitbox.mk

#comment to build to flash
define TO_SRAM 

ifdef TO_SRAM
# NOTE that this program is loaded to SRAM1 (see bitbox memories), thus data is mapped accordingly & load stlink also !
LINKER_SCRIPT=Linker_2Bootloader.ld
# redefinition of stlink load : to sram
stlink: $(NAME).bin
	st-flash write $(NAME).bin 0x20000000
endif 

# the emulator cannot be done for a bootloader
.DEFAULT_GOAL := $(NAME).bin

# extra goals
#.INTERMEDIATE: font.c
font.c: 8x16_437.png
	python mk_font.py $< > $@