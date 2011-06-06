INSTALLDIR = ../../out

CC = gcc

# Flags
CFLAGS += -mthreads -O2 -s
LDFLAGS += $(CFLAGS) 


all: $(INSTALLDIR)/bin $(INSTALLDIR)/lib $(INSTALLDIR)/inc $(TARGET)

$(TARGET): $(OBJS)
	@echo "LD $@" 
	@$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBRARIES)

.c.o:
	@echo "CC $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	@rm -f *~ $(OBJS) *.exe *.dll *.a

$(INSTALLDIR)/bin $(INSTALLDIR)/lib $(INSTALLDIR)/inc:
	@mkdir -p $@