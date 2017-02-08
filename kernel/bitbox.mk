# Bitbox Makefile helper for native targets (also called emulators)

# Variables used
# --------------
#   BITBOX should point to the base bitbox source dir (where this file is)

#   NAME : name of the project
#   GAME_C_FILES c files of the project
#   GAME_C_OPTS : C language options. Those will be used for the ARM game as well as the emulator.

#	DEFINES : C defines, will be added as -Dxxx to GAME_C_OPTS, you can use either
#       VGA_MODE=xxx : define a vga mode.
#		  they can be used to define specific kernel resolution. Usable values are
#         320x240, 384x271, 400x300, 640x480, 800x600 (see kconf.h) 
#         See also PAL_MODE for specific 15kHz modes in kconf_pal.h
#       VGA_BPP=8 or 16 (default) : use a 8bpp mode (for micro, emulated for bibox by kernel)
#       PROFILE		- enable profiling in kernel (red line / pixels onscreen)

# Other Makefile flags :
#  Those flags include a number of support C files to the build. They also export the flags as C defines
#		NO_USB,       - when you don't want to use USB input related function)
#		NO_AUDIO      - disable sound support
#		USE_SDCARD,   - when you want to use SDcard+fatfs support 
#

# More arcane options :
#     USE_SD_SENSE  - enabling this will disable being used on rev2 !
#     DISABLE_ESC_EXIT - for the emulator only, disable quit when pressing ESC
#     KEYB_FR       - enable AZERTY keybard mapping

HOST = $(shell uname)

# export variables to submakes
export 

# default : build bitbox + sdl binaries
all: emu bitbox $(EXTRA_FILES)

# --- option-only targets (independent from target)

# get canonical Bitbox path
BITBOX:=$(realpath $(BITBOX))

BUILD_DIR := build

# --- Helpers

SUBMAKE_ARM    = $(MAKE) -f $(BITBOX)/kernel/bitbox_arm.mk 
SUBMAKE_POSIX  = $(MAKE) -f $(BITBOX)/kernel/bitbox_posix.mk 

# Build the project for the given target
bitbox emu sdl test pal micro debug debug-micro stlink stlink-micro stlink-pal: $(GAME_C_FILES)

bitbox:
	$(SUBMAKE_ARM) BOARD='bitbox'
pal: 
	$(SUBMAKE_ARM) BOARD='pal'
micro:
	$(SUBMAKE_ARM) BOARD='micro'
emu sdl: 
	$(SUBMAKE_POSIX) TYPE='sdl'
test: 
	$(SUBMAKE_POSIX) TYPE='test'
	./$(NAME)_test

debug: 
	$(SUBMAKE_ARM) BOARD='bitbox' debug
debug-micro: 
	$(SUBMAKE_ARM) BOARD='micro' debug
	
# using st-flash
stlink: 
	$(SUBMAKE_ARM) BOARD='bitbox' stlink
stlink-micro: 
	$(SUBMAKE_ARM) BOARD='micro'  stlink
stlink-pal: 
	$(SUBMAKE_ARM) BOARD='pal'    stlink
	
# using dfu util
dfu: 
	$(SUBMAKE_ARM) BOARD='bitbox' dfu
dfu-micro: micro
	$(SUBMAKE_ARM) BOARD='micro' dfu


# double colon to allow extra cleaning
clean::
	rm -rf $(BUILD_DIR) $(NAME)_*.bin $(NAME)_*.elf $(NAME)_sdl $(NAME)_test

.PHONY: clean stlink-pal dfu-micro stlink-micro debug-micro stlink dfu emu micro bitbox pal debug test
