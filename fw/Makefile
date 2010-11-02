# Makefile for compiling 

# Optimization level, put in comment for debugging
OPTIMIZATION = -Os


TARGET = flash

# Output file basename
OUTPUT = uspi-$(TARGET)

# Output directories
BIN = ../out/fw 
OBJ = obj-$(TARGET)

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------

ARM=C:/Keil/ARM/

# Compilation tools
CC = $(ARM)/bin/armcc

# Flags
INCLUDES = -I $(ARM)/inc -I $(ARM)/INC/Atmel/SAM7S
DEFINES = -DFLASH -D__MICROLIB


CFLAGS = $(INCLUDES) $(DEFINES) -c --cpu ARM7TDMI -g -O3 -Otime --apcs=interwork 
ASFLAGS = 
LDFLAGS = 

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------

# Objects built from C source files
C_OBJECTS = main.o

# Objects built from Assembly source files
#ASM_OBJECTS = board_cstartup.o
#ASM_OBJECTS += cp15_asm.o

# Append OBJ and BIN directories to output filename
OUTPUT := $(BIN)/$(OUTPUT)

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

all: $(BIN) $(OBJ) $(MEMORIES)

$(BIN) $(OBJ):
	mkdir $@

define RULES
C_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(C_OBJECTS))
ASM_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(ASM_OBJECTS))

$(1): $$(ASM_OBJECTS_$(1)) $$(C_OBJECTS_$(1))
	$(CC) $(LDFLAGS) -T"$(AT91LIB)/boards/$(BOARD)/$(CHIP)/$$@.lds" -o $(OUTPUT)-$$@.elf $$^
	$(OBJCOPY) -O binary $(OUTPUT)-$$@.elf $(OUTPUT)-$$@.bin
	$(SIZE) $$^ $(OUTPUT)-$$@.elf

$$(C_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.c Makefile $(OBJ) $(BIN)
	$(CC) $(CFLAGS) -D$(1) -c -o $$@ $$<

$$(ASM_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.S Makefile $(OBJ) $(BIN)
	$(CC) $(ASFLAGS) -D$(1) -c -o $$@ $$<

debug_$(1): $(1)
	perl ../resources/gdb/debug.pl $(OUTPUT)-$(1).elf

endef

$(foreach MEMORY, $(MEMORIES), $(eval $(call RULES,$(MEMORY))))

clean:
	-rm -f $(OBJ)/*.o $(BIN)/*.bin $(BIN)/*.elf


