TARGET = uspi_i2c.exe
OBJS = main.o
LIBRARIES =-L../libuspi -luspi
INCLUDES =-I../libuspi

include ../common.mak

install: $(INSTALLDIR)/bin $(TARGET) test_i2c.bat
	@cp $(TARGET) $(INSTALLDIR)/bin
	@cp test_i2c.bat $(INSTALLDIR)/bin
