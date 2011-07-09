#include "config.h"
#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */

#ifdef __GNUC__
unsigned IRQ_Stack[IRQ_Stack_Size/4];
unsigned FIQ_Stack[FIQ_Stack_Size/4];
unsigned SVC_Stack[SVC_Stack_Size/4];
unsigned USR_Stack[USR_Stack_Size/4];
//unsigned UND_Stack[UND_Stack_Size/4];
//unsigned ABT_Stack[ABT_Stack_Size/4];
#endif



#if USPI_PLL_CLK<160000000
#define PMC_PLLR_Val	(((USPI_PLL_USBDIV-1)&0x3)<<28) | (((USPI_PLL_MUL-1)&0x7FF)<<16) | ((USPI_PLL_COUNT&0x3F)<<8) | USPI_PLL_DIV
#elif  USPI_PLL_CLK<220000000
#define PMC_PLLR_Val	(((USPI_PLL_USBDIV-1)&0x3)<<28) | (((USPI_PLL_MUL-1)&0x7FF)<<16) | ((USPI_PLL_COUNT&0x3F)<<8) | USPI_PLL_DIV | 2<<14
#else
#error "too much"
#endif

/* Low level init */
void init()
{
#define PMC_MUL (0x7FF<<16)
	//Setup RSTC
	AT91C_BASE_RSTC->RSTC_RMR = RSTC_MR_Val;

	// Setup WDT
	AT91C_BASE_WDTC->WDTC_WDMR=WDT_MR_Val;

	// Setup Main Oscillator
	AT91C_BASE_PMC->PMC_MOR = PMC_MOR_Val;
	// Wait until Main Oscillator is stablilized
	if(PMC_MOR_Val & AT91C_PMC_MOSCS)
		while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));
	//Setup the PLL
#if USPI_PLL_MUL
	AT91C_BASE_PMC->PMC_PLLR = PMC_PLLR_Val;
	//Wait until PLL is stabilized
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));
#endif
	// Select Clock
	if ( (PMC_MCKR_Val & AT91C_PMC_CSS) == AT91C_PMC_CSS_MAIN_CLK ) // Main Clock Selected
		AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_Val & AT91C_PMC_CSS;
	else if ((PMC_MCKR_Val & AT91C_PMC_CSS) == AT91C_PMC_CSS_PLL_CLK) // PLL  Clock Selected
		AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_Val & AT91C_PMC_PRES;
	while(! (AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

	AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_Val;
	while(! (AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

#ifndef FLASH
	//remap
	AT91C_BASE_MC->MC_RCR=1;
#endif
}

