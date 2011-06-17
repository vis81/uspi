#include "intc.h"
#include "config.h"
#include "assert.h"
#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */

typedef struct _INTCDESC{
	AT91PS_AIC	pAIC;
	U32			IRQ[32];
}INTCDESC,*PINTCDESC;

INTCDESC gIntc;

void irqc ()
{
	int irqline=*AT91C_AIC_ISR;
	((void (*) (void))(*AT91C_AIC_IVR))();
	//SANITY_CHECK(irqline<32);
	//gIntc.IRQ[irqline]++;	
	*AT91C_AIC_EOICR=0;
}



void spi_irq (void);
void pio_irq (void);

void fiqc (void)
{
	unsigned pending=*AT91C_AIC_IPR;
#ifdef PIO_FIQ
	if(pending&(1<<AT91C_ID_PIOA))
		pio_irq();
#endif
#ifdef SPI_FIQ
	if(pending&(1<<AT91C_ID_SPI))
		spi_irq();
#endif
}

void intc_default()
{
	while(1);
}

void intc_spurious()
{
	while(1);
}

void intc_init(){
	int i;
	gIntc.pAIC=AT91C_BASE_AIC;
	gIntc.pAIC->AIC_IDCR=0xFFFFFFFF;
	for(i=0;i<32;i++){
		gIntc.pAIC->AIC_SVR[i] = (unsigned long) intc_default;
		gIntc.IRQ[i]=0;
	}
	gIntc.pAIC->AIC_SVR[AT91C_ID_FIQ] = (unsigned long) fiqc;
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_FIQ] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;
	AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_FIQ);

	gIntc.pAIC->AIC_SPU=(unsigned)intc_spurious;
}

void intc_connect(U8 line, U32 attr, irq_func handler, BOOL FIQ)
{
	gIntc.pAIC->AIC_IDCR = (1<<line);
	gIntc.pAIC->AIC_ICCR= (1<<line);
	gIntc.pAIC->AIC_SVR[line]=(unsigned int)handler;
	gIntc.pAIC->AIC_SMR[line]=attr;
	if(FIQ)
		gIntc.pAIC->AIC_FFER=(1<<line);
	gIntc.pAIC->AIC_IECR= (1<<line);
}
