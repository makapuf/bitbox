NAME=bitbox-mod
GAME_C_FILES = main.c lib/mod/mod32.c
include $(BITBOX)/kernel/bitbox.mk
NO_USB=1
main.c: data.h

.DELETE_ON_ERROR: data.h
data.h: $(wildcard mods/*.mod)
	echo $^
	python $(BITBOX)/lib/resources/embed.py $^ > $@

clean::
	rm -f data.h 
