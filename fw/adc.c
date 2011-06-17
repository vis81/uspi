#include "adc.h"
#include "timer.h" 
#include "pio.h"
#include "spi.h"
#include "assert.h"
#include "usb.h"


/*************************************************************************
				PRIVATE VARS
**************************************************************************/
const static U32 nDRDY_mask[3]={ADC_nDRDY0,ADC_nDRDY1,ADC_nDRDY2};
ADCDESC gAdcDesc;




/*************************************************************************
				PRIVATE FUNCTIONS DECLARATION
**************************************************************************/
void adc_sync_cb(U32 time);
void adc_pio_cb(U32 pin,U32 value);
void adc_spi_cb(void);

void pwm_init(void);
void pwm_start(void);
void pwm_stop(void);



/*************************************************************************
				FUNCTION DEFINITIONS
**************************************************************************/

void adc_init()
{
	pio_init();
	spi_init();
	//nSYNC/nPD
	pio_set_output(ADC_nSYNC,0,FALSE,FALSE);
			
	//ADC_nDRDY
	pio_set_input(ADC_nDRDY0,FALSE,FALSE);
	pio_set_input(ADC_nDRDY1,FALSE,FALSE);
	pio_set_input(ADC_nDRDY2,FALSE,FALSE);
	gAdcDesc.state=ADC_IDLE;
#ifdef DEBUG
	pwm_init();
#endif
}

void adc_start(int spi,int drdy, int numdevices, ADC_DONE_CB cb)
{
	TRACE_INFO("ADC START %u\n",spi);
	pio_clr(ADC_nSYNC);
	gAdcDesc.spi=spi;
	gAdcDesc.drdy=drdy;
	gAdcDesc.adccount=numdevices;
	gAdcDesc.cb=cb;
	gAdcDesc.evt_cnt=(U32)-1;
	gAdcDesc.spi_ovrun=0;
	gAdcDesc.state=ADC_SYNC;
	spi_setup_xfer(spi,numdevices*3,adc_spi_cb);
	tm_start(1000000,adc_sync_cb);
}


void adc_sync_cb(U32 time)
{
	TRACE_INFO("ADC SYNC\n");
	SANITY_CHECK(gAdcDesc.state==ADC_SYNC);
	
	tm_stop();
	gAdcDesc.state=ADC_PIO;
	pio_start(nDRDY_mask[gAdcDesc.drdy],adc_pio_cb);
	pio_set(ADC_nSYNC);	
	
#ifdef DEBUG
	pwm_start();
#endif
}

void adc_pio_cb(U32 pin,U32 value)
{
	if(!value)
	{
		gAdcDesc.evt_cnt++;
		if(gAdcDesc.state==ADC_PIO)
		{
			//initiate SPI transfer
			gAdcDesc.state=ADC_SPI;
			gAdcDesc.adcbuf[gAdcDesc.buf_wr].time=gAdcDesc.evt_cnt;
			spiisr_read(gAdcDesc.adcbuf[gAdcDesc.buf_wr].value);
		}else{	
			gAdcDesc.spi_ovrun++;
		}
	}
}


void adc_spi_cb(void)
{
	if(gAdcDesc.state==ADC_SPI)
	{
		gAdcDesc.buf_wr=(gAdcDesc.buf_wr+1)%ADC_BUFSIZE;
		SANITY_CHECK(gAdcDesc.buf_wr!=gAdcDesc.buf_rd);
		gAdcDesc.state=ADC_PIO;
	}
	else
		SANITY_CHECK(gAdcDesc.state==ADC_IDLE);
}




void adc_stop(void)
{
	TRACE_INFO("ADC STOP\n");
	pio_stop();
	gAdcDesc.buf_wr=gAdcDesc.buf_rd=0;
	gAdcDesc.cb=NULL;
	gAdcDesc.state=ADC_IDLE;
#ifdef DEBUG
	pwm_stop();
#endif
}


BOOL adc_task(void)
{
	if(gAdcDesc.buf_wr!=gAdcDesc.buf_rd)
	{
		SANITY_CHECK(gAdcDesc.cb);
		while(gAdcDesc.buf_rd!=gAdcDesc.buf_wr)
		{
			ADC_BUF* pAdcBuf=&gAdcDesc.adcbuf[gAdcDesc.buf_rd];
			gAdcDesc.cb(pAdcBuf->time,pAdcBuf->value);
			gAdcDesc.buf_rd=(gAdcDesc.buf_rd+1)%ADC_BUFSIZE;
		}
		return TRUE;
	}
	return FALSE;
}

#ifdef DEBUG

AT91PS_PWMC pPwm=(AT91PS_PWMC)AT91C_BASE_PWMC;

void pwm_init()
{
	// Enable PWMC peripheral clock
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PWMC;
	pPwm->PWMC_CH[0].PWMC_CMR=	
#ifdef LOWFREQ
					AT91C_PWMC_CPRE&0x4 | 	//MCK/16
#else
					AT91C_PWMC_CPRE&0x3 | 	//MCK/8
#endif
					AT91C_PWMC_CALG&0 | 	//left aligned
					AT91C_PWMC_CPOL ; 		//starts from high
	pPwm->PWMC_CH[0].PWMC_CPRDR=192;//clock multiplier
	pPwm->PWMC_CH[0].PWMC_CDTYR=2;//duty cycle		

	//setup PIO pins as peripheral
	pio_set_peripheral(AT91C_PA0_PWM0,0,TRUE);
	//set outputs	
	AT91C_BASE_PIOA->PIO_OER=AT91C_PA0_PWM0;
}

void pwm_start()
{
	pPwm->PWMC_ENA=AT91C_PWMC_CHID0;
}

void pwm_stop()
{
	pPwm->PWMC_DIS=AT91C_PWMC_CHID0;
}

#endif

