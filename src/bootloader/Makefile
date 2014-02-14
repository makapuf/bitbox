NAME=sdtest

C_FILES = stm32f4_discovery_sdio_sd.c stm32f4_lowlevel.c ff.c diskio.c main.c 
C_FILES += stm32fxxx_it.c stm32f4xx_dma.c stm32f4xx_gpio.c stm32f4xx_sdio.c  stm32f4xx_rcc.c startup.c misc.c

MCU = cortex-m4
FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__FPU_USED=1
OPT = -O3 -falign-functions=16 -fno-inline -fomit-frame-pointer

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

DEFINES = -DARM_MATH_CM4 
SOURCE_DIR=.

# USB defines
DEFINES += -DUSE_USB_OTG_HS -DUSE_STDPERIPH_DRIVER -DUSE_EMBEDDED_PHY -DUSE_USB_OTG_FS

OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o) $(S_FILES:%.s=$(BUILD_DIR)/%.o) 
BUILD_DIR = build

C_OPTS = -std=c99 \
		-mthumb \
		-mcpu=cortex-m4 \
		-I../Libraries/CMSIS/Include \
		-I../lib/ \
		-g \
		-Werror \
        -O3 \
        -mlittle-endian 
#                 -funroll-loops \
#                -fplan9-extensions


ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS) -mthumb -mcpu=cortex-m4 -nostartfiles -Wl,-T,../Linker.ld,--gc-sections

LIBS =	-lm

all: $(NAME).bin 

stlink: $(NAME).bin
	#arm-eabi-gdb $(NAME).elf --eval-command="target ext :4242"
	st-flash write $(NAME).bin 0x08000000

clean:
	rm -rf $(BUILD_DIR) $(NAME).elf $(NAME).bin $(NAME)_emu 

$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin

$(NAME).elf: $(OBJS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .o .c .S .s

$(BUILD_DIR)/%.o: $(LIB_SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@


$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

debug: $(NAME).elf
	arm-none-eabi-gdbtui $(NAME).elf \
	--eval-command="target extended-remote :4242"