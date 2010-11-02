#include <string.h>
#include <stdio.h>
#include "pio.h"
#include "assert.h"

AT91PS_PIO gPIO = AT91C_BASE_PIOA;

extern AT91S_AIC 	*pAIC;
void spi_isr (void);

PIO_DONE_CB pioCB;


void pio_irq (void)
#ifndef PIO_FIQ
__irq
#endif
{
	U32 status=gPIO->PIO_ISR&gPIO->PIO_IMR;
	if (status) 
	{
		U32 value=gPIO->PIO_PDSR&gPIO->PIO_IMR;
		SANITY_CHECK(pioCB);
		pioCB(status,value);
		AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_PIOA);
	}
	*AT91C_AIC_EOICR = 0;                     /* End of Interrupt          */
}

void pio_init(){
	
	*AT91C_PMC_PCER = (1 << AT91C_ID_PIOA);

	pioCB=NULL;

	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_PIOA] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 6;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_PIOA] = (unsigned long) pio_irq;
#ifdef PIO_FIQ
	pAIC->AIC_FFER=(1<<AT91C_ID_PIOA);
#endif
	AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_PIOA);
}


void pio_start(U32 mask,PIO_DONE_CB cb)
{
	SANITY_CHECK(!pioCB);
	pioCB=cb;
	gPIO->PIO_IER=mask;
}

void pio_stop()
{
	gPIO->PIO_IDR=0xFFFFFFFF;
	pioCB=NULL;
}

void pio_set_peripheral(
    unsigned int maskA,
    unsigned int maskB,
    unsigned char enablePullUp)
{
    // Disable interrupts on the pin(s)
    gPIO->PIO_IDR = maskA|maskB;

    // Enable the pull-up(s) if necessary
    if (enablePullUp) 
		gPIO->PIO_PPUER = maskA|maskB;
    else
		gPIO->PIO_PPUDR = maskA|maskB;

    // Configure pin
    gPIO->PIO_ASR = maskA;
	gPIO->PIO_BSR = maskB;
    gPIO->PIO_PDR = maskA|maskB;
}


void pio_set_output(
    unsigned int mask,
    unsigned char defaultValue,
    unsigned char enableMultiDrive,
    unsigned char enablePullUp)
{
	// Disable interrupts
	gPIO->PIO_IDR = mask;

	// Enable pull-up(s) if necessary
	if (enablePullUp)
		gPIO->PIO_PPUER = mask;
	else
		gPIO->PIO_PPUDR = mask;

	// Enable multi-drive if necessary
	if (enableMultiDrive)
		gPIO->PIO_MDER = mask;
	else
		gPIO->PIO_MDDR = mask;

	// Set default value
	if (defaultValue)
		gPIO->PIO_SODR = mask;
	else
		gPIO->PIO_CODR = mask;

	// Configure pin(s) as output(s)
	gPIO->PIO_OER = mask;
	gPIO->PIO_PER = mask;
}



void pio_set_input(
    unsigned int mask,
    unsigned char enablePullUp,
    unsigned char enableFilter)
{
	// Disable interrupts
	gPIO->PIO_IDR = mask;

	// Enable pull-up(s) if necessary
	if (enablePullUp)
		gPIO->PIO_PPUER = mask;
	else 
		gPIO->PIO_PPUDR = mask;

	// Enable filter(s) if necessary
	if (enableFilter)
		gPIO->PIO_IFER = mask;
	else
		gPIO->PIO_IFDR = mask;

	// Configure pin as input
	gPIO->PIO_ODR = mask;
	gPIO->PIO_PER = mask;
}


void pio_set(unsigned int mask)
{
    gPIO->PIO_SODR = mask;
}


void pio_clr(unsigned int mask)
{
    gPIO->PIO_CODR = mask;
}

unsigned char pio_get_out(unsigned int mask)
{
	return ((gPIO->PIO_ODSR&mask)!=0);
}

unsigned char pio_get_in(unsigned int mask)
{
	return ((gPIO->PIO_PDSR&mask)!=0);
}