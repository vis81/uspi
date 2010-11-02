#ifndef PIO_H
#define PIO_H
#include "type.h"
#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */


typedef void (*PIO_DONE_CB) (U32 mask, U32 value);


void pio_set_peripheral(
    unsigned int maskA,
    unsigned int maskB,
    unsigned char enablePullUp);

void pio_set_output(
    unsigned int mask,
    unsigned char defaultValue,
    unsigned char enableMultiDrive,
    unsigned char enablePullUp);

void pio_set_input(
    unsigned int mask,
    unsigned char enablePullUp,
    unsigned char enableFilter);

void pio_set(unsigned int mask);

void pio_clr(unsigned int mask);

unsigned char pio_get_out(unsigned int mask);

unsigned char pio_get_in(unsigned int mask);


void pio_init(void);

void pio_start(U32 mask,PIO_DONE_CB cb);

void pio_stop(void);

BOOL pio_task(void);



#endif //#ifndef PIO_H

