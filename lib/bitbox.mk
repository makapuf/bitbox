# Bitbox Makefile helper.

# BITBOX environment variable should point to the base bitbox source  dir (where this file is)
# DEFINES in outside makefile
#   NAME : name of the project
#   GAME_C_FILES c files of the project
#   GAME_BIN_FILES : files to embed as part of the main binary ROM. Note: you can use GAME_BIN_FILES=$(wildcard data/*) 

#   GAME_C_OPTS : C language options. Those will be used for the ARM game as well as the emulator.
#	C DEFINES : defined in DEFINED Makefile Variable. (will be added as -Dxxx to GAME_C_OPTS, you can use either)
#       PROFILE		- enable profiling (red line / pixels onscreen)
#       VGA_MODExxx : define a vga mode. 

#		  they can be used to define specific kernel resolution. 
#   	  In particular, define one of VGAMODE_640, VGAMODE_800, VGAMODE_320 or VGA_640_OVERCLOCK
#   	  to set up a resolution in the kernel (those will be used in kconf.h)
#
#   Specific Makefile flags : 
#         NO_USB,       - when you don't want to use USB input related function), also exported as C define
#		  NO_AUDIO      - no sound support (exported as C define)
#         USE_SDCARD,   - when you want to use or compile SDcard or fatfs related functions in the game (export to C)
#    
#         USE_ENGINE,   - when you want to use the engine. 
#         USE_SAMPLER
#         USE_CHIPTUNES 
#
#         MICROKERNEL : switch the kernel interface will be the micro kernel interface (8bpp colors and sound) 
#             Note that a micro game can be compiled for multiples target boards : micro, bitbox, sdl or test.
#			  It's just that the exposed capabilities are different. Exported to C defines when enabled.
# 
#		  AUTOKERNEL : use micro kernel on micro, and bitbox kernel interface on other targets. 
#				will define MICROKERNEL in C on bitbox micro board only.
#		     
#   Simple mode related : 
#        VGA_SIMPLE_MODE=0 .. 12 (see simple.h for modes)

# More arcane options : 
#     USE_SD_SENSE  - enabling this will disable being used on rev2 !
#     DISABLE_ESC_EXIT - for the emulator only, disable quit when pressing ESC
#     KEYB_FR       - enable AZERTY keybard mapping


# just the names of the targets in a generic way
BITBOX_TGT:=$(NAME).elf
MICRO_TGT:=$(NAME)_micro.elf
SDL_TGT:=$(NAME) 
TEST_TGT:=$(NAME)_test

all: $(SDL_TGT) $(BITBOX_TGT:%.elf=%.bin) 

# --- option-only targets (independent from target)

BUILD_DIR := build

VPATH=.:$(BITBOX)/lib:$(BITBOX)/lib/StdPeriph

INCLUDES=-I$(BITBOX)/lib/ -I$(BITBOX)/lib/cmsis -I$(BITBOX)/lib/StdPeriph

# language specific (not specific to target)
C_OPTS = -std=c99 -g -Wall -ffast-math -fsingle-precision-constant -ffunction-sections -fdata-sections -funroll-loops -fomit-frame-pointer 

LD_FLAGS = -Wl,--gc-sections 
AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

# functional defines for all targets. -D will be expanded after. 
#DEFINES = 

# --- Engines (not target specific)

GAME_C_FILES += evt_queue.c 

# - tiles & sprites
ifdef USE_ENGINE
GAME_C_FILES +=  blitter.c blitter_btc.c blitter_sprites.c blitter_tmap.c
endif

# - simple modes
ifdef VGA_SIMPLE_MODE
DEFINES += VGA_SIMPLE_MODE=$(VGA_SIMPLE_MODE)
GAME_C_FILES += simple.c fonts.c
endif # vga kernel mode defined in kconf.h 

# - simple sampler
ifdef USE_SAMPLER
GAME_C_FILES += sampler.c
endif

# - chiptune engine
ifdef USE_CHIPTUNE 
$(warning the chiptune engine is about to change. Please change to the chiptune.h file)
GAME_C_FILES += chiptune_engine.c chiptune_player.c
endif

# -- Target-specifics 
ifdef AUTOKERNEL
$(MICRO_TGT): DEFINES += MICROKERNEL
endif 


$(BITBOX_TGT): DEFINES += BOARD_BITBOX
$(MICRO_TGT):  DEFINES += BOARD_MICRO

CORTEXM4F=-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -march=armv7e-m -mlittle-endian -nostartfiles
$(BITBOX_TGT) $(MICRO_TGT): CC=arm-none-eabi-gcc
$(BITBOX_TGT) $(MICRO_TGT): C_OPTS += -O3 $(CORTEXM4F)
$(BITBOX_TGT) $(MICRO_TGT): LD_FLAGS += $(CORTEXM4F)

ifdef LINKER_RAM
$(BITBOX_TGT): LD_FLAGS+=-Wl,-T,$(BITBOX)/lib/Linker_bitbox_ram.ld
dfu stlink: FLASH_START = 0x20000000
else ifdef NO_BOOTLOADER 
$(BITBOX_TGT): LD_FLAGS+=-Wl,-T,$(BITBOX)/lib/Linker_bitbox_raw.ld
dfu stlink: FLASH_START = 0x08000000
else
$(BITBOX_TGT): LD_FLAGS+=-Wl,-T,$(BITBOX)/lib/Linker_bitbox_loader.ld
dfu stlink: FLASH_START = 0x08004000
endif 

$(MICRO_TGT): LD_FLAGS+=-Wl,-T,$(BITBOX)/lib//Linker_micro.ld
dfu-micro stlink-micro: FLASH_START = 0x08000000

HOST = $(shell uname)
ifeq ($(HOST), Haiku)
  HOSTLIBS =
else
  HOSTLIBS = -lm -lc -lstdc++
endif
$(SDL_TGT) $(TEST_TGT): CC=gcc
$(SDL_TGT) $(TEST_TGT): DEFINES += EMULATOR
$(SDL_TGT): C_OPTS += -Og 
$(SDL_TGT): C_OPTS += $(shell sdl-config --cflags)
$(SDL_TGT): HOSTLIBS += $(shell sdl-config --libs)

KERNEL_SDL+=emulator.c
KERNEL_TEST+=tester.c
KERNEL_MICRO+=board_micro.c startup.c bitbox_main.c
KERNEL_BITBOX+=board.c startup.c system.c bitbox_main.c

# -- Optional AND target specific

ifndef NO_VGA
  KERNEL_MICRO += vga_micro.c
  KERNEL_BITBOX += new_vga.c
  ifdef MICROKERNEL
    KERNEL_BITBOX += micro_palette.c
    KERNEL_SDL += micro_palette.c
  	DEFINES += MICROKERNEL
  endif 
else 
  DEFINES += NO_VGA
endif

# fatfs related files
SDCARD_FILES := fatfs/stm32f4_lowlevel.c fatfs/stm32f4_discovery_sdio_sd.c fatfs/ff.c fatfs/diskio.c 
SDCARD_FILES += stm32f4xx_sdio.c stm32f4xx_rcc.c stm32f4xx_gpio.c stm32f4xx_dma.c misc.c
ifdef USE_SDCARD
DEFINES += USE_SDCARD
KERNEL_BITBOX += $(SDCARD_FILES)
KERNEL_MICRO += $(SDCARD_FILES)
endif 

# USB defines
ifdef NO_USB
DEFINES += NO_USB
else 
$(BITBOX_TGT) $(MICRO_TGT): DEFINES += USE_STDPERIPH_DRIVER
USB_FILES := usb_bsp.c usb_core.c usb_hcd.c usb_hcd_int.c \
	usbh_core.c usbh_hcs.c usbh_stdreq.c usbh_ioreq.c \
	usbh_hid_core.c usbh_hid_keybd.c usbh_hid_mouse.c usbh_hid_gamepad.c \
	usbh_hid_parse.c misc.c
KERNEL_BITBOX += $(USB_FILES)
KERNEL_MICRO += $(USB_FILES)
endif

ifdef NO_AUDIO
DEFINES+=NO_AUDIO
else
KERNEL_BITBOX += audio_bitbox.c
KERNEL_MICRO += audio_micro.c
endif

# --- binaries as direct object linking + binaries.h from all data in /data directory (if present)

# see http://stackoverflow.com/questions/17265950/linking-arbitrary-data-using-gcc-arm-toolchain
DATA_OBJ:= $(patsubst %,$(BUILD_DIR)/data/%.o,$(GAME_BINARY_FILES))
$(BUILD_DIR)/data/%.o: data/%
	@mkdir -p $(dir $@)
	objcopy -I binary -O elf32-little $^ $@

# ----------------------------------------------

GAME_C_FILES+=$(GAME_BINARY_FILES:%=$(BUILD_DIR)/%.c)

$(BUILD_DIR)/binaries.h: $(GAME_BINARY_FILES)
#	_binary_<plop>_start , _binary_<plop>_end , _binary_<plop>_len
	# should transform it using make syntax.
	@mkdir -p $(dir $@)
	echo "// AUTO GENERATED BY bitbox.mk DO NOT MODIFY " > $@
	echo "// -- binaries " > $@
	echo $^ | sed s/[/\.]/_/g | sed "s/ /\n/g" | sed "s/.*/extern const unsigned char \0[];/" >> $@ 
	echo "// -- lengths " >> $@
	echo $^ | sed s/[/\.]/_/g | sed "s/ /\n/g" | sed "s/.*/extern const unsigned int \0_len;/">> $@ 

$(BUILD_DIR)/%.c: % 
	@mkdir -p $(dir $@)
	$(info * embedding $^ as $@)
	xxd -i $^ | sed "s/unsigned/const unsigned/" > $@

# --- Compilation pattern rules

ALL_CFLAGS = $(DEFINES:%=-D%) $(C_OPTS) $(INCLUDES) $(GAME_C_OPTS)

# must put 4 rules and not only once since multi target pattern rules are special :)
$(BUILD_DIR)/bitbox/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@
$(BUILD_DIR)/micro/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@
$(BUILD_DIR)/sdl/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@
$(BUILD_DIR)/test/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $^ $@
	chmod -x $@

# --- Targets 

$(SDL_TGT): $(GAME_C_FILES:%.c=$(BUILD_DIR)/sdl/%.o) $(KERNEL_SDL:%.c=$(BUILD_DIR)/sdl/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS)

$(TEST_TGT): $(GAME_C_FILES:%.c=$(BUILD_DIR)/test/%.o) $(KERNEL_TEST:%.c=$(BUILD_DIR)/test/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS) 

$(BITBOX_TGT): $(GAME_C_FILES:%.c=$(BUILD_DIR)/bitbox/%.o) $(KERNEL_BITBOX:%.c=$(BUILD_DIR)/bitbox/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS) 
	chmod -x $@

$(MICRO_TGT): $(GAME_C_FILES:%.c=$(BUILD_DIR)/micro/%.o) $(KERNEL_MICRO:%.c=$(BUILD_DIR)/micro/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(HOSTLIBS) 
	chmod -x $@

# --- Helpers

test: $(NAME)_test
	./$(NAME)_test

debug: $(BITBOX_TGT)
	arm-none-eabi-gdb $^ --eval-command="target extended-remote :4242"

# using dfu util	
dfu: $(NAME).bin
	dfu-util -D $< --dfuse-address $(FLASH_START) -a 0

stlink: $(NAME).bin
	st-flash write $^ $(FLASH_START)

debug-micro: $(MICRO_TGT)
	arm-none-eabi-gdb $^ --eval-command="target extended-remote :4242"

stlink-micro: $(NAME)_micro.bin
	st-flash write $^ $(FLASH_START)
# using dfu util	
dfu-micro: $(NAME)_micro.bin
	dfu-util -D $< --dfuse-address $(FLASH_START) -a 0

# double colon to allow extra cleaning
clean::
	rm -rf $(BUILD_DIR) $(MICRO_TGT) $(BITBOX_TGT) $(NAME).bin $(SDL_TGT) $(TEST_TGT)
