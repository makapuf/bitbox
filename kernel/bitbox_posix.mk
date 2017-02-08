# Bitbox Makefile helper for native targets (also called emulators)

# TARGETS 
# --------
# $NAME_$BOARD (default), clean
 
# Variables used (export them)
# --------------
#   TYPE= sdl | test
#   BITBOX NAME GAME_C_FILES DEFINES (VGA_MODE, VGA_BPP, ...)
#   GAME_C_OPTS DEFINES NO_USB NO_AUDIO USE_SDCARD
# More arcane defines :
#   USE_SD_SENSE DISABLE_ESC_EXIT KEYB_FR

HOST = $(shell uname)
$(warning compiling $(NAME) with c files $(GAME_C_FILES))
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
else ifeq ($(TYPE), test)
else 
  $(error unknown type $(TYPE) defined, please use sdl or test)
endif

KERNEL := main_$(TYPE).c micro_palette.c

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
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@
	@echo CC $<

# --- Autodependecies (headers...)
-include $(BUILD_DIR)/$(TYPE)/*.d

# --- link
$(NAME)_$(TYPE): $(GAME_C_FILES:%.c=$(BUILD_DIR)/%.o) $(KERNEL:%.c=$(BUILD_DIR)/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS)

# --- Helpers

# double colon to allow extra cleaning
clean::
	rm -rf $(BUILD_DIR) $(NAME)_$(TYPE)

.PHONY: clean 
