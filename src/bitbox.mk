# DEFINE in outside makefile
#   NAME : name of the project
#   GAME_C_FILES c files of the project
#   
# Update 
#   DEFINES with whatever defines are needed : -DPLATFORM_BITBOX
#   CFLAGS

#NAME = shoot
#GAME_C_FILES = test_data.c object.c test_game.c
#GAME_H_FILES = test_data.h kernel.h object.h test_object.h

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

DEFINES =	-DARM_MATH_CM4 -DOVERCLOCK -DAUDIO -DGAMEPAD
#-DPLATFORM_BITBOX

C_OPTS = -std=c99 \
		-mthumb \
		-mcpu=cortex-m4 \
		-I../Libraries/CMSIS/Include \
		-I../lib/ \
		-g \
		-Werror \
		-O3 \
		-mlittle-endian \
		-funroll-loops \
		-fplan9-extensions

LIBS =	-lm

SOURCE_DIR =.
LIB_SOURCE_DIR =../lib
BUILD_DIR =build
#LIB_FILES = ../lib/object.c

KERNEL_FILES = startup.c system.c new_vga.c bitbox_main.c gamepad.c audio.c

C_FILES = $(LIB_FILES) $(GAME_C_FILES) $(KERNEL_FILES)
		
S_FILES =

OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o) $(S_FILES:%.S=$(BUILD_DIR)/%.o)


ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS) -mthumb -mcpu=cortex-m4 -nostartfiles -Wl,-T,../Linker.ld,--gc-sections
#-specs Terrible.specs

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@

all: $(NAME).bin $(NAME)_emu

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
	st-flash write $(NAME).bin 0x08000000

clean:
	rm -rf $(BUILD_DIR) $(NAME).elf $(NAME).bin $(NAME)_emu 

$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin

$(NAME).elf: $(OBJS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .o .c .S

$(BUILD_DIR)/%.o: $(LIB_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@


$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)


$(NAME)_emu: $(GAME_C_FILES) ../lib/emulator.c 
	gcc -O1 $(GAME_C_FILES) -I ../lib/ ../lib/emulator.c  -g -Wall -std=c99 -lm `sdl-config --cflags --libs` -o $(NAME)_emu

