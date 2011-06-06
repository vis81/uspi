TARGET = uspi.dll
OBJS = uspi.o
CFLAGS = -shared
LDFLAGS = -Wl,--out-implib,libuspi.a
INCLUDES = -I. -I../../libusb-win32-bin-1.2.1.0/include
LIBRARIES =-L../../libusb-win32-bin-1.2.1.0/lib/gcc -lusb

include ../common.mak

$(TARGET) libuspi.a: $(OBJS)
	@echo "LD $@" 
	@$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBRARIES)


install: $(INSTALLDIR)/bin $(INSTALLDIR)/lib $(INSTALLDIR)/inc $(TARGET) libuspi.a
	@cp $(TARGET) $(INSTALLDIR)/bin
	@cp libuspi.a $(INSTALLDIR)/lib
	@cp uspi.h $(INSTALLDIR)/inc
