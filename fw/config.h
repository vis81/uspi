#ifndef _USPI_CFG_H_
#define _USPI_CFG_H_

#define DEBUG
#define TRACE_LEVEL TRACE_LEVEL_INFO /*DEBUG  INFO WARNING  ERROR NO_TRACE*/      


//CLOCK
#define MCK						47923200 /* Main CLK frequency */

//TIMER
#define RTT_FREQ				10		/* MIPS measurement  frequency */

//USB cfg
#define VID						0xe463
#define PID						0x0007
#define MANUFACTURER 			L"Tomek"
#define PRODUCT 				L"Uspi"
#define VERSION					L"Test_17_10a"

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
#define PIO_FIQ

//SPI
#define SPI_FIQ
#define	SPI_INIT_LB				FALSE
#define	SPI_INIT_CPOL_HIGH 		FALSE
#define SPI_INIT_PHASE_RISING	TRUE
#define SPI_INIT_SCBR			4
#define	SPI_INIT_DLYBS			1
#define	SPI_INIT_DLYBCT			0

#endif
