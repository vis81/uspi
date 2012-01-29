#ifndef _TWI_H_
#define _TWI_H_
#include "type.h"
#include "config.h"

void twi_init (void);
void twi_setclk(int hz);
int twi_readbyte(int SlaveAddr,
                       int IntAddr,
                       int IntAddrSize,
                       char *data);

int twi_writebyte(int SlaveAddr,
                        int IntAddr,
                        int IntAddrSize,
                        char *data);



#endif
