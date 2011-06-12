MEMORY=flash
DEBUG=true

# Output directories
INSTALLDIR = ../out
BINDIR = $(INSTALLDIR)/fw
SRCDIR = .

# Compilation tools
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy

# Flags
INCLUDES = -I.
ALLFLAGS = -mcpu=arm7tdmi -fno-common -fshort-wchar $(OPTIMIZATION) 

CFLAGS += -g $(ALLFLAGS) $(INCLUDES)
ASFLAGS += $(CFLAGS) -D__ASSEMBLY__
LDFLAGS += $(ALLFLAGS) -nostartfiles -Wl,--gc-sections

ifeq ($(DEBUG),true)
CFLAGS += -O3 
# -DDEBUG
ifeq ($(MEMORY),ram)
TARGET=ram_debug
else
TARGET=flash
CFLAGS += -DFLASH
endif
else
CFLAGS += -O2
ifeq ($(MEMORY),ram)
TARGET=ram_release
else
TARGET=flash_release
CFLAGS += -DFLASH
endif
endif

LD_SCRIPT=$(SRCDIR)/$(MEMORY).lds
OBJDIR=$(TARGET)_obj

#-------------------------------------------------------------------------------
#		Files
#-------------------------------------------------------------------------------

# Objects built from C source files
C_OBJECTS = adc.o chanmgr.o cmd.o intc.o board.o
C_OBJECTS += main.o pio.o spi.o time.o usb.o

# Objects built from Assembly source files
ASM_OBJECTS = start.o

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

all: $(OBJDIR) $(OBJDIR)/$(TARGET).elf

$(BINDIR) $(OBJDIR):
	mkdir -p $@


C_OBJECTS := $(addprefix $(OBJDIR)/, $(C_OBJECTS))
ASM_OBJECTS := $(addprefix $(OBJDIR)/, $(ASM_OBJECTS))

$(OBJDIR)/$(TARGET).elf : $(ASM_OBJECTS) $(C_OBJECTS) $(LD_SCRIPT)
	$(CC) $(LDFLAGS) -T$(LD_SCRIPT) -Wl,-Map,$(OBJDIR)/$(TARGET).map -o $(OBJDIR)/$(TARGET).elf $(ASM_OBJECTS) $(C_OBJECTS)
	$(OBJCOPY) -O binary $@ $(OBJDIR)/$(TARGET).bin
	$(SIZE) $(ASM_OBJECTS) $(C_OBJECTS) $(OBJDIR)/$(TARGET).elf

$(C_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c  $(SRCDIR)/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(ASM_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.S $(SRCDIR)/*.h
	$(CC) $(ASFLAGS) -c -o $@ $<


clean:
	-rm -rf $(OBJDIR)

install: $(BINDIR)
	@cp $(OBJDIR)/$(TARGET).bin $(BINDIR)
