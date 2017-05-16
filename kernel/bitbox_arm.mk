# Bitbox ARM Makefile helper.

# TARGETS 
# --------
# clean, stlink, dfu, debug, $NAME_$BOARD.elf/bin (default)

# Variables used (export them)
# --------------
#   BOARD= bitbox | micro | pal 
#   TYPE= sdl | test
#   BITBOX NAME GAME_C_FILES DEFINES (VGA_MODE, VGA_BPP, ...)
#   GAME_C_OPTS NO_USB NO_AUDIO USE_SDCARD
# More arcane defines :
#   USE_SD_SENSE DISABLE_ESC_EXIT KEYB_FR

ifndef BOARD
	$(error "gotta define BOARD")
endif 

HOST = $(shell uname)

all: $(NAME)_$(BOARD).bin

# get canonical Bitbox path
BITBOX:=$(realpath $(BITBOX))

BUILD_DIR := build/$(BOARD)

VPATH=.:$(BITBOX)/kernel:$(BITBOX)/kernel/StdPeriph:$(BITBOX)/

INCLUDES=-I$(BITBOX)/kernel/ \
    -I$(BITBOX)/kernel/cmsis \
    -I$(BITBOX)/kernel/StdPeriph \
    -I$(BITBOX)

# language specific (not specific to target)
C_OPTS = -std=c99 -g -Wall -ffast-math -fsingle-precision-constant \
    -fsigned-char -ffunction-sections -fdata-sections -funroll-loops \
    -fomit-frame-pointer 
    #\-flto 

LD_FLAGS = -Wl,--gc-sections

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

# -- Target-specifics (uppercase)
DEFINES += BOARD_$(shell echo ${BOARD} | tr '[:lower:]' '[:upper:]')

CORTEXM4F=-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16\
     -march=armv7e-m -mlittle-endian -nostartfiles
     
CC=arm-none-eabi-gcc
C_OPTS += -Ofast $(CORTEXM4F)
LD_FLAGS += $(CORTEXM4F)

ifeq ($(BOARD) , micro)
  LD_FLAGS+=-Wl,-T,$(BITBOX)/kernel/Linker_micro.ld
  dfu stlink: FLASH_START = 0x08000000
else
  ifdef LINKER_RAM
    LD_FLAGS+=-Wl,-T,$(BITBOX)/kernel/Linker_bitbox_ram.ld 
    FLASH_START = 0x20000000
  else ifdef NO_BOOTLOADER
    LD_FLAGS+=-Wl,-T,$(BITBOX)/kernel/Linker_bitbox_raw.ld
    FLASH_START = 0x08000000
  else
    LD_FLAGS+=-Wl,-T,$(BITBOX)/kernel/Linker_bitbox_loader.ld
    FLASH_START = 0x08004000
  endif
endif 

LIBS = -lm

KERNEL += board_$(BOARD).c \
    startup.c \
    bitbox_main.c \
    vga_$(BOARD).c \
    micro_palette.c

# -- Optional features

# Fatfs 
ifdef USE_SDCARD
SDCARD_FILES := fatfs/stm32f4_lowlevel.c fatfs/stm32f4_discovery_sdio_sd.c fatfs/ff.c fatfs/diskio.c
SDCARD_FILES += stm32f4xx_sdio.c stm32f4xx_gpio.c stm32f4xx_dma.c misc.c

DEFINES += USE_SDCARD USE_STDPERIPH_DRIVER
KERNEL += $(SDCARD_FILES)
endif

# USB defines
ifdef NO_USB
DEFINES += NO_USB
else
DEFINES += USE_STDPERIPH_DRIVER
USB_FILES := usb_bsp.c usb_core.c usb_hcd.c usb_hcd_int.c \
	usbh_core.c usbh_hcs.c usbh_stdreq.c usbh_ioreq.c \
	usbh_hid_core.c usbh_hid_keybd.c usbh_hid_mouse.c usbh_hid_gamepad.c \
	usbh_hid_parse.c misc.c
KERNEL += $(USB_FILES)
endif

# Audio
ifeq ($(filter NO_AUDIO,$(DEFINES)),)  
KERNEL += audio_$(BOARD).c
endif

# -- C compilation
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(DEFINES:%=-D%) $(C_OPTS) $(INCLUDES) $(GAME_C_OPTS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@
	@echo CC-ARM $< 

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $^ $@
	chmod -x $@

# --- Autodependecies (headers...)
-include $(BUILD_DIR)/*.d

# -- linking 
$(NAME)_$(BOARD).elf : $(GAME_C_FILES:%.c=$(BUILD_DIR)/%.o) $(KERNEL:%.c=$(BUILD_DIR)/%.o)
	$(CC) $(LD_FLAGS) $^ -o $@ $(LIBS)
	chmod -x $@

# --- Helpers

debug: $(NAME)_$(BOARD).bin
	arm-none-eabi-gdb $^ --eval-command="target extended-remote :4242"

# Build the project for the given target

# using dfu util
dfu: $(NAME)_$(BOARD).bin
	dfu-util -D $< --dfuse-address $(FLASH_START) -a 0

stlink: $(NAME)_$(BOARD).elf
	st-flash write $^ $(FLASH_START)

# double colon to allow extra cleaning
clean::
	rm -rf $(NAME)_$(BOARD).elf $(NAME)_$(BOARD).bin build/

.PHONY: clean stlink dfu debug

