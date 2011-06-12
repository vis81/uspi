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
//   <h> Main Oscillator
//     <o1.0>      MOSCEN: Main Oscillator Enable
//     <o1.1>      OSCBYPASS: Oscillator Bypass
//     <o1.8..15>  OSCCOUNT: Main Oscillator Startup Time <0-255>
//   </h>
//   <h> Phase Locked Loop (PLL)
//     <o2.0..7>   DIV: PLL Divider <0-255>
//     <o2.16..26> MUL: PLL Multiplier <0-2047>
//                 <i> PLL Output is multiplied by MUL+1
//     <o2.14..15> OUT: PLL Clock Frequency Range
//                 <0=> 80..160MHz  <1=> Reserved
//                 <2=> 150..220MHz <3=> Reserved
//     <o2.8..13>  PLLCOUNT: PLL Lock Counter <0-63>
//     <o2.28..29> USBDIV: USB Clock Divider
//                 <0=> None  <1=> 2  <2=> 4  <3=> Reserved
//   </h>
//   <o3.0..1>   CSS: Clock Source Selection
//               <0=> Slow Clock
//               <1=> Main Clock
//               <2=> Reserved
//               <3=> PLL Clock
//   <o3.2..4>   PRES: Prescaler
//               <0=> None
//               <1=> Clock / 2    <2=> Clock / 4
//               <3=> Clock / 8    <4=> Clock / 16
//               <5=> Clock / 32   <6=> Clock / 64
//               <7=> Reserved
#define PMC_PLLR_Val	0x10191C05
#define PMC_MCKR_Val	0x00000007
#define PMC_MOR_Val		0x00000601

//WatchDog
//   <o1.0..11>  WDV: Watchdog Counter Value <0-4095>
//   <o1.16..27> WDD: Watchdog Delta Value <0-4095>
//   <o1.12>     WDFIEN: Watchdog Fault Interrupt Enable
//   <o1.13>     WDRSTEN: Watchdog Reset Enable
//   <o1.14>     WDRPROC: Watchdog Reset Processor
//   <o1.28>     WDDBGHLT: Watchdog Debug Halt
//   <o1.29>     WDIDLEHLT: Watchdog Idle Halt
//   <o1.15>     WDDIS: Watchdog Disable
#define WDT_MR_Val		0x00008000

//Reset controller
//   <o1.0>     URSTEN: User Reset Enable
//              <i> Enables NRST Pin to generate Reset
//   <o1.8..11> ERSTL: External Reset Length <0-15>
//              <i> External Reset Time in 2^(ERSTL+1) Slow Clock Cycles
#define RSTC_MR_Val		0xA5000401

//Stack sizes
#define IRQ_Stack_Size	0x200
#define FIQ_Stack_Size	0x200
#define SVC_Stack_Size	0x280
#define USR_Stack_Size	0x200
#define UND_Stack_Size	0x40
#define ABT_Stack_Size	0x40

#endif
