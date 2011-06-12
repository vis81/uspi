PROG = uspi
TRGTS = app fw
APPS = uspi_conv uspi_check libuspi uspi_test labview

$(PROG): $(TRGTS)

all: $(TRGTS)

$(APPS):
	@cd app ; echo "MAKE $@";make $@


$(TRGTS):
	@cd ./$@ ; echo "MAKE $@";make all

clean :
	@for d in $(TRGTS); do (cd $$d; echo "CLEAN $$d";make clean ); done

install:
	@for d in $(TRGTS); do (cd $$d; echo "INSTALL $$d"; make install ); done

restart:
	tools/devcon restart "USB\Vid_e463&Pid_0007"

program_jlink:
	sam-ba.exe  "\\jlink\\ARM0" AT91SAM7S64-EK scripts/prog_flash.tcl 1>program.log 2>&1

.PHONY: $(TRGTS)
