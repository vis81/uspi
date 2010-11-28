#ifndef _CHANMGR_H_
#define _CHANMGR_H_
#include "commands.h"

#define CHAN_BUF_SIZE 64


typedef struct _CHAN_DESC {
	BOOL started;
	U8 spi;
	U8 drdy;
	U8 adcnum;
	U8 hsize;
	U8 eventsize;
	EP_DESC* pEpDesc;
	U8 	Packet[CHAN_BUF_SIZE];
	
	//Stats
	U32 PktSent;
	U32 UsbOvflw;
} CHAN_DESC;

extern CHAN_DESC gUspiChannelDesc;


int chan_start(U8 spi,U8 drdy,U8 adcnum,U8 hsize);
int chan_stop(void);

#endif
