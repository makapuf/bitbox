NAME=sawteeth
GAME_CPP_FILES = $(NAME).cpp source/txt.cpp source/song.cpp \
    source/ins_part_channel.cpp source/player.cpp source/insply.cpp \
    source/lfo.cpp source/wave.cpp
GAME_C_FILES = syscalls.c
GAME_BINARY_FILES = rawData
GAME_C_OPTS = -D_XOPEN_SOURCE=700 -Iheaders

include $(BITBOX)/lib/bitbox.mk
