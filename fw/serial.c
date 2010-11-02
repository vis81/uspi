/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include <stdio.h>
#include "serial.h"
#include "type.h"
#include "trace.h"


#define BRD  (MCK/16/UART_BAUD_RATE)                    /* Baud Rate Divisor */

typedef struct
{
    unsigned int put;
    unsigned int get;
    char         buf[UART_BUF_SIZE];
} UART_Buffer;

UART_Buffer TX_Buffer;
UART_Buffer* pTxBuf=&TX_Buffer;
AT91S_USART * pUSART = AT91C_BASE_US0;      /* Global Pointer to USART0 */


static void uartisr_txhandler(void);


__irq void uart_isr(void)
{
    uartisr_txhandler();
	*AT91C_AIC_EOICR = 0;
}

static void uartisr_txhandler()
{
	if(!(pUSART->US_CSR & AT91C_US_TXRDY))
		return;
    if(pTxBuf->put!=pTxBuf->get)
    {
		pUSART->US_THR = pTxBuf->buf[pTxBuf->get++];
        if(pTxBuf->get>=UART_BUF_SIZE)
            pTxBuf->get=0;
    }
	else
	   pUSART->US_IDR=AT91C_US_TXRDY;
}

int uartisr_write (const void* buf, unsigned nbytes)
{
    char *p=(char*)buf;
	U32	put;

	while(nbytes--)
	{
		put = pTxBuf->put;
		if (++put >= UART_BUF_SIZE)
			put = 0;

		if (put != pTxBuf->get)
		{
			pTxBuf->buf[pTxBuf->put] = *p++;
			pTxBuf->put = put;
		}else
			break;
	}
    //kick transfer
	pUSART->US_IER=AT91C_US_TXRDY;
    uartisr_txhandler();
    return nbytes;
}


void uart_init (void) {                   /* Initialize Serial Interface */

	*AT91C_PMC_PCER = (1 << AT91C_ID_US0);    /* Enable Clock for USART0 */

	*AT91C_PIOA_PDR = AT91C_PA5_RXD0 |        /* Enable RxD0 Pin */
						AT91C_PA6_TXD0;         /* Enalbe TxD0 Pin */

	pUSART->US_CR = AT91C_US_RSTRX |          /* Reset Receiver      */
							AT91C_US_RSTTX |          /* Reset Transmitter   */
							AT91C_US_RXDIS |          /* Receiver Disable    */
							AT91C_US_TXDIS;           /* Transmitter Disable */

	pUSART->US_MR = AT91C_US_USMODE_NORMAL |  /* Normal Mode */
							AT91C_US_CLKS_CLOCK    |  /* Clock = MCK */
							AT91C_US_CHRL_8_BITS   |  /* 8-bit Data  */
							AT91C_US_PAR_NONE      |  /* No Parity   */
							AT91C_US_NBSTOP_1_BIT;    /* 1 Stop Bit  */

	pUSART->US_BRGR = BRD;                    /* Baud Rate Divisor */

	// Disable the interrupt first
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_US0;

	// Configure mode and handler
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_US0] = AT91C_AIC_PRIOR_LOWEST|
											AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_US0] = (unsigned int) uart_isr;

	// Clear interrupt
	AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_US0;

	pUSART->US_IER=AT91C_US_TXRDY;

	//Enable interrupt
	AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_US0;
	
	pUSART->US_CR = AT91C_US_TXEN;            /* Transmitter Enable  */
	
	TRACE_INFO("UART init done %u bps\n",UART_BAUD_RATE);
	
	
}


int kputchar (int ch)  {                    /* Write character to Serial Port */
	//uart_write(&ch,1);
	//uartisr_write(&ch,1);
	return ch;
}
