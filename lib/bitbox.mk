# DEFINE in outside makefile
#   NAME : name of the project
#   GAME_C_FILES c files of the project
#   
# Update 
#   DEFINES with whatever defines are needed with -DXYZ
#   CFLAGS

# specific flags : use audio, use USB, use uSD, ... (?)

#NAME = shoot
#GAME_C_FILES = test_data.c object.c test_game.c $(NAME)
#GAME_H_FILES = test_data.h kernel.h object.h test_object.h

#MCU  = cortex-m4
#FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__FPU_USED=1
#OPT = -O3 -falign-functions=16 -fno-inline -fomit-frame-pointer -funroll-loops

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

DEFINES = -DARM_MATH_CM4 -DOVERCLOCK  -DPROFILE 
#-DAUDIO

# USB defines
ifdef NO_USB
DEFINES += -DNO_USB
else
DEFINES += -DUSE_USB_OTG_HS -DUSE_EMBEDDED_PHY -DUSE_USB_OTG_FS
endif

DEFINES += -DUSE_STDPERIPH_DRIVER 

C_OPTS = -std=c99 \
	-mthumb \
	-mcpu=cortex-m4 \
	-Ilib/ \
	-Ilib/CMSIS/Include \
	-Ilib/StdPeriph \
	-Werror \
        -O3 \
        -mlittle-endian \
        -g \
        -fomit-frame-pointer 
        #-funroll-all-loops \

LIBS =	-lm

SOURCE_DIR =.
DATA_DIR=data
BUILD_DIR =build
LIB_SOURCE_DIR =lib
LIB_STD_SOURCE_DIR =lib/StdPeriph

# replace with linker_raw if you want to overwrite bootloader
LINKER_SCRIPT = lib/Linker_loader.ld
FLASH_START = 0x08004000
#FLASH_START = 0x08000000
#LINKER_SCRIPT = lib/Linker_raw.ld

KERNEL_FILES = startup.c system.c \
	new_vga.c bitbox_main.c audio.c \
	evt_queue.c\
	stm32f4xx_gpio.c \
	stm32f4xx_rcc.c \
	stm32f4xx_tim.c \
	misc.c 

ifndef NO_USB
	KERNEL_FILES += usb_bsp.c \
	usb_core.c \
	usb_hcd.c \
	usb_hcd_int.c \
	usbh_core.c \
	usbh_hcs.c \
	usbh_hid_core.c \
	usbh_hid_keybd.c \
	usbh_hid_mouse.c \
	usbh_hid_gamepad.c \
	usbh_ioreq.c \
	usbh_stdreq.c \
	stm32fxxx_it.c 
	#usbh_usr.c 
endif 

# fatfs related files
KERNEL_FILES += fatfs/stm32f4_lowlevel.c fatfs/stm32f4_discovery_sdio_sd.c stm32f4xx_sdio.c stm32f4xx_dma.c fatfs/ff.c fatfs/diskio.c

# engine related
ifdef USE_ENGINE
ENGINE_FILES = lib/blitter.c 
endif

C_FILES = $(LIB_FILES) $(GAME_C_FILES) $(KERNEL_FILES) $(ENGINE_FILES)
S_FILES = memcpy-armv7m.S


OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o) $(S_FILES:%.S=$(BUILD_DIR)/%.o) $(GAME_BINARY_FILES:%=$(BUILD_DIR)/%_dat.o) 


ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS) $(GAME_C_OPTS)
ALL_LDFLAGS = $(LD_FLAGS) -mthumb -mcpu=cortex-m4 -nostartfiles -Wl,-T,$(LINKER_SCRIPT),--gc-sections
#-specs Terrible.specs

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

all: $(NAME).bin $(NAME)_emu $(EXTRA_FILES)

upload: $(NAME).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg \
	-c init -c "reset halt" -c "stm32f2x mass_erase 0" \
	-c "flash write_bank 0 $(NAME).bin 0" \
	-c "reset run" -c shutdown

debug: $(NAME).elf
	arm-none-eabi-gdb $(NAME).elf \
	--eval-command="target extended-remote :4242"

stlink: $(NAME).bin
	#arm-eabi-gdb $(NAME).elf --eval-command="target ext :4242"
	st-flash write $(NAME).bin $(FLASH_START)

# double colon to allow extra cleaning
clean::
	rm -rf $(BUILD_DIR) $(NAME).elf $(NAME).bin $(NAME)_emu *.btc 

$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin

$(NAME).elf: $(OBJS) 
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .o .c .S

$(BUILD_DIR)/%.o: $(LIB_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_STD_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

# ---------- data embedding 

$(BUILD_DIR)/%_dat.o: $(SOURCE_DIR)/%
	@mkdir -p $(dir $@)
	xxd -i $< | sed "s/unsigned/const unsigned/" > $(BUILD_DIR)/$*.c
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c build/$*.c -o $@

$(SOURCE_DIR)/%.btc: $(SOURCE_DIR)/%.png
	python ../scripts/btc4.py $<

$(SOURCE_DIR)/%.spr: $(SOURCE_DIR)/%.png
	python ../scripts/sprite_encode1.py $< $@

# ---------------------------------

$(BUILD_DIR)/%.o: $(LIB_SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)


$(NAME)_emu: $(GAME_C_FILES) lib/emulator.c $(ENGINE_FILES) $(GAME_BINARY_FILES)
	gcc -Og -DEMULATOR $(GAME_C_FILES) $(ENGINE_FILES) $(GAME_BINARY_FILES:%=$(BUILD_DIR)/%.c) $(GAME_C_OPTS) -Ilib/ lib/emulator.c  -g -Wall -std=c99 -lm `sdl-config --cflags --libs` -o $(NAME)_emu

