#ifndef INTC_H
#define INTC_H
#include "type.h"

typedef void (*irq_func) (void);


void intc_init(void);
void intc_connect(U8 line, U32 attr, irq_func handler, BOOL FIQ);


#endif
