#ifndef _ADC_H_
#define _ADC_H_
#include "type.h"

#define ADC_BUFSIZE 10


#define ADC_nSYNC	AT91C_PIO_PA16
#define ADC_nDRDY0	AT91C_PIO_PA17
#define ADC_nDRDY1	AT91C_PIO_PA18
#define ADC_nDRDY2	AT91C_PIO_PA21

enum ADC_STATE{
	ADC_IDLE,
	ADC_SYNC,
	ADC_PIO,
	ADC_SPI,
	ADC_DONE
};

typedef struct _ADC_BUF{
	U32 time;
	U8 value[9];
}ADC_BUF;

typedef void (*ADC_DONE_CB) (U32 event,U8* data);


typedef struct _ADCDESC{
	U8 spi;
	U8 drdy;
	U8 adccount;
	U8 state;
	ADC_DONE_CB cb;
	
	U8 buf_wr;
	U8 buf_rd;
	ADC_BUF adcbuf[ADC_BUFSIZE];
	
	U32 evt_cnt;
	U32 evt_done;
	U32 spi_ovrun;
}ADCDESC,*PADCDESC;

extern ADCDESC gAdcDesc;


void adc_init(void);
void adc_start(int adc,int drdy, int numdevices, ADC_DONE_CB cb);
void adc_stop(void);

BOOL adc_task(void);


#endif
