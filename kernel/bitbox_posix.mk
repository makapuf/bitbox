# Bitbox Makefile helper for native targets (also called emulators)

# TARGETS 
# --------
# $NAME_$BOARD (default), clean
 
# Variables used
# --------------
#   BITBOX should point to the base bitbox source dir (where this file is)
#   TYPE= sdl | test

#   NAME : name of the project
#   GAME_C_FILES c files of the project
#   GAME_C_OPTS : C language options. Those will be used for the ARM game as well as the emulator.

#	DEFINES : C defines, will be added as -Dxxx to GAME_C_OPTS, you can use either
#       VGA_MODE=xxx : define a vga mode.
#		  they can be used to define specific kernel resolution. Usable values are
#         320x240, 384x271, 400x300, 640x480, 800x600 (see kconf.h) 
#         See also PAL_MODE for specific 15kHz modes in kconf_pal.h
#       VGA_BPP=8 or 16 (default) : use a 8bpp mode (for micro, emulated on kernel)
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
$(warning compiling $(NAME))
DEFINES += EMULATOR 

# get canonical Bitbox path
BITBOX:=$(realpath $(BITBOX))

BUILD_DIR := build/$(TYPE)

VPATH=.:$(BITBOX)/kernel:$(BITBOX)/

INCLUDES=-I$(BITBOX)/kernel/  -I$(BITBOX)

# language specific (not specific to target)
C_OPTS = -std=c99 -g -Wall \
    -ffast-math -fsingle-precision-constant -fsigned-char \
    -ffunction-sections -fdata-sections -funroll-loops -fomit-frame-pointer

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

# -- Target-specifics
ifeq ($(HOST), Haiku)
  HOSTLIBS =
else
  HOSTLIBS = -lm -lc -lstdc++
endif
ifeq ($(HOST), Darwin)
  C_OPTS += -O0
  LD_FLAGS = -dead_strip
else
  C_OPTS += -Og
  LD_FLAGS = -Wl,--gc-sections
endif

CC=gcc

ifeq ($(TYPE), sdl)
  C_OPTS   += $(shell sdl-config --cflags)
  HOSTLIBS += $(shell sdl-config --libs)
else ifeq ($TYPE), test)
else 
  $(error unknown type defined, please use sdl or test)
endif

KERNEL   += main_$(TYPE).c micro_palette.c

# -- Optional features 

ifdef USE_SDCARD
  DEFINES += USE_SDCARD 
endif
ifdef NO_USB
  DEFINES += NO_USB
endif 
ifdef NO_AUDIO
  DEFINES += NO_AUDIO
endif

# --- Compilation

ALL_CFLAGS = $(DEFINES:%=-D%) $(C_OPTS) $(INCLUDES) $(GAME_C_OPTS)

# must put different rules and not only once since multi target pattern rules are special :)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

# --- Autodependecies (headers...)
-include $(BUILD_DIR)/$(TYPE)/*.d

# --- Targets
$(NAME)_$(TYPE): $(GAME_C_FILES:%.c=$(BUILD_DIR)/%.o) $(KERNEL:%.c=$(BUILD_DIR)/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS)

# --- Helpers

# double colon to allow extra cleaning
clean::
	rm -rf $(BUILD_DIR) $(NAME)_$(TYPE)

.PHONY: clean 
