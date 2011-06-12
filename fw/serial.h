#ifndef _SERIAL_H_
#define _SERIAL_H_
#include "config.h"

void uart_init (void);
//int __swi(0) uart_write(const void* buf, unsigned nbytes);
int kputchar (int ch);
#endif
