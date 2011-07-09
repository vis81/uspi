#include "AT91SAM7S64.H"                    
#include "timer.h"
#include "serial.h"
#include "intc.h"

volatile unsigned long timeval=0;
volatile unsigned long realtime=0;
volatile int temp;

volatile BOOL gTimerEvent;
static TM_DONE_CB tmCB;
AT91S_AIC 	*pAIC 	=AT91C_BASE_AIC;
AT91S_TC	*pTimer0=AT91C_BASE_TC0;
AT91S_TC	*pTimer1=AT91C_BASE_TC1;
AT91_REG* ppivr=&AT91C_BASE_TC1->TC_SR;



void tm_tc_start(U8 TimerID, irq_func isr, unsigned isr_mode ,unsigned clock,unsigned div)
{
	AT91S_TC	*pTimer=NULL;
	U32 dummy;
	pTimer=(AT91S_TC*) ((U32)AT91C_BASE_TC0+0x40*(TimerID-AT91C_ID_TC0));


	// First, enable the clock of the TIMER
	*AT91C_PMC_PCER = (1 << TimerID);

	// Disable the clock and the interrupts
	pTimer->TC_CCR = AT91C_TC_CLKDIS;
	pTimer->TC_IDR = 0xFFFFFFFF;

	dummy = pTimer->TC_SR; // clear status bit
	dummy = dummy; // suppress warning

	pTimer->TC_CMR = clock | AT91C_TC_CPCTRG; // set mode of the Timer Counter
	pTimer->TC_CCR = AT91C_TC_CLKEN; // enable the clock

	pTimer->TC_RC = div;

	intc_connect(TimerID,isr_mode, isr,FALSE);
	pTimer->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
	//* Start timer0
	pTimer->TC_CCR = AT91C_TC_SWTRG ;

}


void tm_rt_isr (void)
{
	realtime++;
	temp=pTimer0->TC_SR;
}
	

void tm_start_rt (void) {
	realtime=0;
	tm_tc_start(AT91C_ID_TC0, tm_rt_isr, 
		AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL| AT91C_AIC_PRIOR_LOWEST,
		AT91C_TC_CLKS_TIMER_DIV5_CLOCK, USPI_MCK/1024/RTT_FREQ);

}


void tm_isr (void)
{
	timeval++;
	gTimerEvent=TRUE;
	temp=pTimer1->TC_SR;
	pTimer1->TC_CCR = AT91C_TC_CLKDIS;
	pTimer1->TC_IDR = 0xFFFFFFFF;
}

void tm_start(unsigned freq,TM_DONE_CB cb)
{
	tmCB=cb;
	timeval=0;
	gTimerEvent=TRUE;
	tm_tc_start(AT91C_ID_TC1, tm_isr, 
			AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL| AT91C_AIC_PRIOR_HIGHEST,
			AT91C_TC_CLKS_TIMER_DIV1_CLOCK,USPI_MCK/2/freq);
}

void tm_stop()
{
	// Disable the clock and the interrupts
	pTimer1->TC_CCR = AT91C_TC_CLKDIS;
	pTimer1->TC_IDR = 0xFFFFFFFF;
	tmCB=NULL;
	gTimerEvent=FALSE;
}

BOOL tm_task(void)
{
	//check which channels to wake-up
	if(gTimerEvent)
	{
		U32 time=timeval;
		gTimerEvent=FALSE;
		if(tmCB)
			tmCB(time);
		return TRUE;
	}
	return FALSE;
}
