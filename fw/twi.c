#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include "twi.h"
#include "trace.h"
#include "assert.h"
#include "pio.h"
#include "intc.h"


AT91PS_TWI pTwi=AT91C_BASE_TWI;


int twi_writebyte(int SlaveAddr,
                        int IntAddr,
                        int IntAddrSize,
                        char *data)
{
    unsigned int end = 0, error = 0, status, retries=100000;

    /* Enable Master Mode */
    pTwi->TWI_CR = AT91C_TWI_MSEN ;

    /* Set the TWI Master Mode Register */
    pTwi->TWI_MMR =  (SlaveAddr<<16 | IntAddrSize<<8) & ~AT91C_TWI_MREAD;

    /* Set TWI Internal Address Register if needed */
    pTwi->TWI_IADR = IntAddr;

    /* Write the data to send into THR. Start conditionn DADDR and R/W bit
       are sent automatically */
    pTwi->TWI_THR = *data;

    /* NACK errata handling */
    /* Do not poll the TWI_SR */
    /* Wait 3 x 9 TWCK pulse (max) 2 if IADRR not used, before reading TWI_SR */
    /* From 400Khz down to 1Khz, the time to wait will be in ¦s range.*/
    /* In this example the TWI period is 1/400KHz */
    //AT91F_TWI_WaitMicroSecond (40) ;

    while (!end)
    {
      status = AT91C_BASE_TWI->TWI_SR;
      if ((status & AT91C_TWI_NACK) == AT91C_TWI_NACK)
      {
        error=1;
        goto out;
      }
    /*  Wait for the Transmit ready is set */
      if ((status & AT91C_TWI_TXRDY) == AT91C_TWI_TXRDY)
        end=1;
      if( --retries==0){
	error=2;
	goto out;
	}
	
    }

out:
    /* Wait for the Transmit complete is set */
    status = AT91C_BASE_TWI->TWI_SR;
    while (!(status & AT91C_TWI_TXCOMP))
      status = AT91C_BASE_TWI->TWI_SR;

	pTwi->TWI_CR = AT91C_TWI_MSDIS;
    return error;
}



int twi_readbyte(int SlaveAddr,
                       int IntAddr,
                       int IntAddrSize,
                       char *data)
{
    unsigned int status,end=0, error=0, retries=100000;

    /* Enable Master Mode */
    pTwi->TWI_CR = AT91C_TWI_MSEN ;

    /* Set the TWI Master Mode Register */
    AT91C_BASE_TWI->TWI_MMR =  SlaveAddr<<16 | IntAddrSize<<8 | AT91C_TWI_MREAD;

    /* Set TWI Internal Address Register if needed */
    AT91C_BASE_TWI->TWI_IADR = IntAddr;

    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_STOP;

    /* NACK errata handling */
    /* Do not poll the TWI_SR */
    /* Wait 3 x 9 TWCK pulse (max) 2 if IADRR not used, before reading TWI_SR */
    /* From 400Khz down to 1Khz, the time to wait will be in ¦s range.*/
    /* In this example the TWI period is 1/400KHz */
    //AT91F_TWI_WaitMicroSecond (40) ;

    while (!end)
    {
      status = AT91C_BASE_TWI->TWI_SR;
      if ((status & AT91C_TWI_NACK) == AT91C_TWI_NACK)
      {
        error=1;
        goto out;
      }
    /*  Wait for the receive ready is set */
      if ((status & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY)
        end=1;

      if( --retries==0){
	error=2;
	goto out;
	}

    }

    *(data) = AT91C_BASE_TWI->TWI_RHR;

out:
   /* Wait for the Transmit complete is set */
   status = AT91C_BASE_TWI->TWI_SR;
   while (!(status & AT91C_TWI_TXCOMP))
     status = AT91C_BASE_TWI->TWI_SR;

   pTwi->TWI_CR = AT91C_TWI_MSDIS;

    return error;
}


void twi_setclk(int hz)
{
  unsigned int cldiv,ckdiv=1 ;

  /* CLDIV = ((Tlow x 2^CKDIV) -3) x Tmck */
  /* CHDIV = ((THigh x 2^CKDIV) -3) x Tmck */
  /* Only CLDIV is computed since CLDIV = CHDIV (50% duty cycle) */

  while ( ( cldiv = ( (USPI_MCK/(2*hz))-3 ) / (1<<ckdiv)) > 255 )
   ckdiv++ ;

  AT91C_BASE_TWI->TWI_CWGR =(ckdiv<<16)|((unsigned int)cldiv << 8)|(unsigned int)cldiv  ;
}


void twi_init ()
{
	//Enable peripheral clock
	AT91C_BASE_PMC->PMC_PCER = (1<<AT91C_ID_TWI);
	
	//PIO PINS SETUP FOR SPI BUS
	//setup PIO pins as peripheral
	pio_set_peripheral(AT91C_PA3_TWD|AT91C_PA4_TWCK,0,FALSE);
	//set opendrain
	AT91C_BASE_PIOA->PIO_MDER=AT91C_PA3_TWD|AT91C_PA4_TWCK;

    //* Disable interrupts
	AT91C_BASE_TWI->TWI_IDR = (unsigned int) -1;

    //* Reset peripheral
	AT91C_BASE_TWI->TWI_CR = AT91C_TWI_SWRST;

	twi_setclk(USPI_TWI_CLK_HZ);
}


//PA3 blue TWD               1 SDA
//Pa4	red TCK				   2 SCL
//GND grey - 6 GND
