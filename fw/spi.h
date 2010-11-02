#ifndef _SPI_H_
#define _SPI_H_
#include "type.h"
#include "config.h"

typedef void (*SPI_DONE_CB) ();

typedef enum _spi_mode_t{
    NPCS0,
    NPCS1,
    NPCS2,
    NPCS3,
    SPI_LOOPBACK
}spi_mode_t;


void spi_init (void);

U32 spi_config(BOOL loopback,
	BOOL SpckInactiveHigh,
	BOOL SpckCaptureRising,
	U8 SCBR,
	U8 DLYBS,
	U8 DLYBCT);


void spi_setup_xfer(U8 Spi, U8 xfersize, SPI_DONE_CB cb);

BOOL spi_task(void);
//U32 __swi(4) spi_read(U8* RxData);

U32 spiisr_read(U8* RxData);

#endif
