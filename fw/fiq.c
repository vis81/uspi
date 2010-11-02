#include "fiq.h"
#include "config.h"
#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */


void spi_irq (void);
void pio_irq (void);

void fiq (void) __irq
{
#ifdef PIO_FIQ
	pio_irq();
#endif
#ifdef SPI_FIQ
	spi_irq();
#endif
}


void fiq_init(){
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_FIQ] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_FIQ] = (unsigned long) fiq;
	AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_FIQ);
}
