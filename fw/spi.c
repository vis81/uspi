#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include <stdio.h>
#include "spi.h"
#include "trace.h"
#include "assert.h"
#include "pio.h"


typedef struct _SPILIST{
	struct _SPILIST* Next;
	U8 SpiId;
	U8 Size;
}SPILIST,*PSPILIST;

typedef struct _SPIDESC{
	AT91PS_SPI pSPI;
	SPI_DONE_CB DoneCb;
	U32 SpiID;
	U32 XferSize;
	U32 BytesDone;
	U32 IRQ;
}SPIDESC,*PSPIDESC;


SPIDESC gSpiDesc;

static U32 spimask[5]={0xE0000,0xD0000,0xB0000,0x70000,0xF0000};

static U32 nCS_mask[3]={AT91C_PA11_NPCS0,AT91C_PA9_NPCS1,AT91C_PA10_NPCS2};



void spiisr_start(void);

__inline void spiisr_select(U8 SpiId)
{
	U32 MR=AT91C_BASE_SPI->SPI_MR;
	//U32 mask=(SpiId<<16);
	//U32 mask=spimask[SpiId];
	MR&=~AT91C_SPI_PCS;
	MR|=spimask[SpiId];
	AT91C_BASE_SPI->SPI_MR=MR;

}

void spi_irq (void)
#ifndef SPI_FIQ
__irq 
#endif
{
	U32 status=AT91C_BASE_SPI->SPI_SR&AT91C_BASE_SPI->SPI_IMR;
	
	SANITY_CHECK(!(status&(AT91C_SPI_MODF|AT91C_SPI_OVRES)));
	if(status&AT91C_SPI_ENDRX)
	{
		gSpiDesc.IRQ++;
		gSpiDesc.BytesDone+=gSpiDesc.XferSize;
		AT91C_BASE_SPI->SPI_IDR=AT91C_SPI_ENDRX;// (SPI) End of Receiver Transfer
		gSpiDesc.DoneCb();
	}
	if(status)
		AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_SPI);
	*AT91C_AIC_EOICR = 0;                     /* End of Interrupt */
}



void spi_init ()
{
	gSpiDesc.SpiID=0;

	// Disable the interrupt first
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_SPI;

	//Enable peripheral clock
	AT91C_BASE_PMC->PMC_PCER = (1<<AT91C_ID_SPI);
	
	//PIO PINS SETUP FOR SPI BUS
	
	gSpiDesc.pSPI=AT91C_BASE_SPI;
    
    //SPI CONTROL REGISTER
    AT91C_BASE_SPI->SPI_CR = (AT91C_SPI_SPIEN | AT91C_SPI_SWRST);	
    AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN;
    
	spi_config(SPI_INIT_LB,
				SPI_INIT_CPOL_HIGH,
				SPI_INIT_PHASE_RISING,
				SPI_INIT_SCBR,
				SPI_INIT_DLYBS,
				SPI_INIT_DLYBCT);

	// Configure mode and handler
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SPI] = AT91C_AIC_PRIOR_HIGHEST|
											AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SPI] = (unsigned int) spi_irq;
	// Clear interrupt
	AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_SPI;

	//Enable SPI IRQ
	AT91C_BASE_SPI->SPI_IER=	AT91C_SPI_MODF	|	// (SPI) Mode Fault Error
								AT91C_SPI_OVRES;// (SPI) Overrun Error Status
#ifdef SPI_FIQ
  AT91C_BASE_AIC->AIC_FFER=(1<<AT91C_ID_SPI);
#endif
	
	//Enable interrupt
	AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_SPI;
	
	gSpiDesc.XferSize=9;
	
	
	TRACE_INFO("SPI: Init done\n");
    
}

void spi_setup_xfer(U8 Spi, U8 xfersize, SPI_DONE_CB cb)
{
	U8 i;
	gSpiDesc.DoneCb=cb;
	gSpiDesc.SpiID=Spi;
	gSpiDesc.XferSize=xfersize;

//#ifndef DEBUG
#if 1
	//setup PIO pins as peripheral
	pio_set_peripheral(AT91C_PA12_MISO|AT91C_PA14_SPCK|nCS_mask[Spi],0,FALSE);
	//set outputs	
	AT91C_BASE_PIOA->PIO_OER=AT91C_PA14_SPCK|nCS_mask[Spi];
	//setup MOSI as const 0
	pio_set_input(AT91C_PA13_MOSI,FALSE,FALSE);
#else
	//setup PIO pins as peripheral
	pio_set_peripheral(AT91C_PA13_MOSI|AT91C_PA12_MISO|AT91C_PA14_SPCK|nCS_mask[Spi],0,FALSE);
	//set outputs	
	AT91C_BASE_PIOA->PIO_OER=AT91C_PA13_MOSI|AT91C_PA14_SPCK|nCS_mask[Spi];
#endif
	for(i=0;i<3;i++)
		if(i!=Spi)
			pio_set_input(nCS_mask[i],FALSE,FALSE);
}


U32 spiisr_read(U8* RxData) 
{ //SWI
	static U8 TxData[]={0x12,0x34,0x56,0x78,0x90,0xAA,0xBB,0xCC,0xDD};
	AT91C_BASE_SPI->SPI_RPR = (unsigned int) RxData;
	AT91C_BASE_SPI->SPI_TPR = (unsigned int) TxData;
	spiisr_start();
	return 1; 
}


void spiisr_start() 
{
	spiisr_select(gSpiDesc.SpiID);
	AT91C_BASE_SPI->SPI_RCR = gSpiDesc.XferSize;
	AT91C_BASE_SPI->SPI_TCR = gSpiDesc.XferSize;
	AT91C_BASE_SPI->SPI_IER=  AT91C_SPI_ENDRX;	// (SPI) End of Receiver Transfer
	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_RXTEN;
	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTEN;
}


U32 spi_config(BOOL loopback,
	BOOL SpckInactiveHigh,
	BOOL SpckCaptureRising,
	U8 SCBR,
	U8 DLYBS,
	U8 DLYBCT)
{
	U32 i;
	
	SCBR&=0xFF;
	DLYBS&=0xFF;
	DLYBCT&=0xFF;
	if(!SCBR)SCBR=1;
    //SPI MODE REGISTER
    AT91C_BASE_SPI->SPI_MR = 	AT91C_SPI_MSTR | 	//Master mode
    							AT91C_SPI_PS_FIXED|	//Fixed chip select
    							AT91C_SPI_MODFDIS| //modefault int disable 
    							(0xF<<16)	| 			//Select none
    							(0<<24)	|			//Delay Between Chip Selects
								(loopback?AT91C_SPI_LLB:0);		//loopback mode
	
	for(i=0;i<4;i++)
	{		
		//SPI CHIP SELECT REGISTER 0 
		AT91C_BASE_SPI->SPI_CSR[i] =	(SpckInactiveHigh?AT91C_SPI_CPOL:0)|	//Clock polarity high
										(SpckCaptureRising?AT91C_SPI_NCPHA:0) |	//Clock phase
										
										//AT91C_SPI_CSAAT |	//CS active after transfer!!!!!!!!!!!!!!1
										
										AT91C_SPI_BITS_8|	//8 bits per transfer
										(SCBR<<8)| 			//SCBR	Baud rate divisor
										(DLYBS<<16)|			//DLYBS Delay before SPCK
										(DLYBCT<<24);			//DLYBCT delay between transfers
	}
	TRACE_INFO("SPI: Setup LB=%u, CPOL=%u, NCPHA=%u, SCBR=%u, DLYBS=%u, DLYBCT=%u\n",loopback,
				SpckInactiveHigh,SpckCaptureRising,SCBR,DLYBS,DLYBCT);
	return 0;
}

