#ifndef _USPI_CFG_H_
#define _USPI_CFG_H_

#define DEBUG
#define TRACE_LEVEL TRACE_LEVEL_NO_TRACE /*DEBUG  INFO WARNING  ERROR NO_TRACE*/      


//CLOCK
#define MCK						47923200 /* Main CLK frequency */

//TIMER
#define RTT_FREQ				10		/* MIPS measurement  frequency */

//USB cfg
#define VID						0xe463
#define PID						0x0007
#define MANUFACTURER 			L"Tomek"
#define PRODUCT 				L"Uspi"
#define VERSION					L"Test_13_XX"

//USB
#define USB_EP_NUM				2
#define USB_EP0_BUFSIZE			64
#define MAX_EP0_PAYLOAD_SIZE	64
#define USB_WRITE_OPT 			FALSE
#define CALC_USB_LATENCY		FALSE
#ifdef FLASH
#define USB_EP_BUFSIZE			64*200
#else
#define USB_EP_BUFSIZE			64*20
#endif


//UART
#define UART_BAUD_RATE			115200	/* Baud Rate */
#define UART_BUF_SIZE			50

//PIO
#define PIO_FIQ					TRUE

//SPI							
#define SPI_FIQ					TRUE
#define	SPI_INIT_LB				FALSE
#define SPI_INIT_SCBR			4

//PMC
#define PMC_PLLR_Val	0x10191C05
#define PMC_MCKR_Val	0x00000007
#define PMC_MOR_Val		0x00000601

//WatchDog
#define WDT_MR_Val		0x00008000

//Reset controller
#define RSTC_MR_Val		0xA5000401

#endif
