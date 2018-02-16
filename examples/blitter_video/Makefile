#testkernel makefile
NAME = test_vid
SDK := $(abspath ../../sdk)
GAME_C_FILES = main.c $(SDK)/lib/blitter/blitter.c $(SDK)/lib/blitter/blitter_btc.c
USE_SDCARD = 1
NO_USB = 1
include ../../sdk/kernel/bitbox.mk

# explicit external dependency. download it ?
main.c: video.bin

video.bin:
	wget https://github.com/makapuf/bitbox/releases/download/v0.1/video.zip
	unzip video.zip