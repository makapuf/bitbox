##############################################################################################
#			MAKEFILE for "   " project on STM32F4 Discovery board
#
##############################################################################################
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#		Don't forget to modify DLIBDIR for your configuration
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#
##############################################################################################
# Start of user section
#

####################################### 
# Define project name 
PROJECT        = FATFS
#######################################

HEAP_SIZE      = 4096
STACK_SIZE     = 2048

#
# Define linker script file here
#
LDSCRIPT = cmsis/link.ld
FULL_PRJ = $(PROJECT)


# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = main.c diskio.c ff.c stm32f4_discovery.c stm32f4_discovery_sdio_sd.c stm32f4xx_it.c system_stm32f4xx.c \
stm32f4xx_dma.c stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_sdio.c stm32f4xx_usart.c misc.c

# List ASM source files here
ASRC = cmsis/startup_stm32f4xx.S

# List all user directories here
UINCDIR = cmsis/

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
# OPT = -g
OPT = -Os -falign-functions=16 -fno-inline -fomit-frame-pointer -mthumb

#
# End of user defines
##############################################################################################

##############################################################################################
# Start of default section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc -std=gnu99
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
BIN  = $(CP) -O binary
SZ   = $(TRGT)size

MCU  = cortex-m4

#
# Define FPU settings here
#
FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__FPU_USED=1

# List all default C defines here, like -D_DEBUG=1
DDEFS = -D__arm__ -D__ASSEMBLY__ -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000 -D__FPU_PRESENT=1  -D__FPU_USED=1 -DSTM32F405VG -DUSE_USB_OTG_FS

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = -D__arm__ -D__ASSEMBLY__ -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000 -D__FPU_PRESENT=1  -D__FPU_USED=1 -DSTM32F405VG

# List all default directories to look for include files here
DINCDIR = ../ 

# List the default directory to look for the libraries here
DLIBDIR = C:\GNUToolsARMEmbedded\4.7-2012q4\arm-none-eabi\lib\armv7e-m\fpu

# List all default libraries here
DLIBS = -lgcc -lc -lm

#
# End of default section
##############################################################################################



INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1


ADEFS   = $(DADEFS) $(UADEFS) -D__HEAP_SIZE=$(HEAP_SIZE) -D__STACK_SIZE=$(STACK_SIZE)
OBJS    = $(ASRC:.S=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mthumb -mcpu=$(MCU) $(FPU)

ASFLAGS  = $(MCFLAGS) $(OPT) -g -gdwarf-2 -Wa,-amhls=$(<:.S=.lst) $(ADEFS)

CPFLAGS  = $(MCFLAGS) $(OPT) -gdwarf-2 -Wall -Wstrict-prototypes -fverbose-asm 
CPFLAGS += -ffunction-sections -fdata-sections -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

LDFLAGS  = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--gc-sections $(LIBDIR)

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#--------------------------------------------------------------------------------------------------------------
# makefile rules
#--------------------------------------------------------------------------------------------------------------

all: $(OBJS) $(FULL_PRJ).elf $(FULL_PRJ).bin size


%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.S
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	@echo ' '
	@echo '-----------LINKING-------------- '
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@ 
  
%bin: %elf
	$(BIN) $< $@

size : $(FULL_PRJ).elf
	@echo ' '
	@echo '-------------------------------- '
	@echo 'Invoking: ARM Windows GNU Print Size'
	$(SZ)  --format=berkeley -t $(FULL_PRJ).elf
	@echo 'Finished building: $@'
	@echo ' '

clean:
	rm -f $(OBJS)
	rm -f $(FULL_PRJ).elf
	rm -f $(FULL_PRJ).map
#	rm -f $(FULL_PRJ).bin
	rm -f $(SRC:.c=.c.bak)
	rm -f $(SRC:.c=.lst)
	rm -f $(ASRC:.S=.S.bak)
	rm -f $(ASRC:.S=.lst)
	rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
