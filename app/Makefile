PROG = apps
TRGTS = uspi_conv uspi_check libuspi uspi_test labview

$(PROG): $(TRGTS)

all: $(TRGTS)

$(TRGTS):
	@cd ./$@ ; echo "MAKE $@";make all

clean :
	@for d in $(TRGTS); do (cd $$d; echo "CLEAN $$d";make clean ); done

install:
	@for d in $(TRGTS); do (cd $$d; echo "INSTALL $$d"; make install ); done

.PHONY: $(TRGTS)
