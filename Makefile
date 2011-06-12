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
	sam-ba.exe  "\\jlink\\ARM0" AT91SAM7S64-EK scripts/samba_segger.tcl 1>program.log 2>&1
program:
	openocd --file=scripts/config.ocd  --file=scripts/flash.ocd
test:
	out/bin/uspi_test -spi 0 -drdy 0 -nadc 3 -time 5000 -scbr 16 -loops 1
	out/bin/uspi_check uspi0.dat
	rm uspi0.dat


.PHONY: $(TRGTS)
