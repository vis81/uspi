TARGET = uspi_test.exe
OBJS = main.o
LIBRARIES =-L../libuspi -luspi
INCLUDES =-I../libuspi

include ../common.mak

install: $(INSTALLDIR)/bin $(TARGET) test.bat
	@cp $(TARGET) $(INSTALLDIR)/bin
	@cp test.bat $(INSTALLDIR)/bin
