# 80x25 text
VGA_SIMPLE_MODE=0 

USE_SAMPLER=1
NO_SDCARD=1

NAME = sampler

GAME_C_FILES = test_sampler.c 
GAME_BINARY_FILES = $(wildcard snd/*.raw)

$(warning $(wildcard snd/*.raw) )

# bitbox_simple.mk, will include files by default
include $(BITBOX)/lib/bitbox.mk
