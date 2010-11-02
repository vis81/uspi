#include <string.h>
#include <stdio.h>
#include "usb.h"
#include "chanmgr.h"
#include "spi.h"
#include "timer.h"
#include "commands.h"
#include "trace.h"
#include "assert.h"
#include "adc.h"
#include "pio.h"

CHAN_DESC gUspiChannelDesc;

void chan_adc_cb(U32 event,U8* data);

int chan_start(U8 spi,U8 drdy,U8 adcnum,U8 hsize,U8 dsize)
{
	if(gUspiChannelDesc.started)
		chan_stop();
	
	TRACE_INFO("Channel start\n");
	memset(&gUspiChannelDesc,0,sizeof(gUspiChannelDesc));
	gUspiChannelDesc.pEpDesc=usb_open(1);
	usb_reset_ep(gUspiChannelDesc.pEpDesc);
	gUspiChannelDesc.started=TRUE;
	gUspiChannelDesc.hsize=hsize;
	gUspiChannelDesc.dsize=dsize;
	gUspiChannelDesc.adcnum=adcnum;
	gUspiChannelDesc.drdy=drdy;
	gUspiChannelDesc.spi=spi;
	gUspiChannelDesc.eventsize=adcnum*3;
	adc_start(spi,drdy,adcnum,chan_adc_cb);		
	return 0;
}

int chan_stop()
{
	if(gUspiChannelDesc.started)
	{
		gUspiChannelDesc.started=FALSE;
		adc_stop();
		usb_flush(gUspiChannelDesc.pEpDesc);
		TRACE_INFO("Channel stop\n");
	}
	return 0;
}

void chan_adc_cb(U32 event,U8* data)
{	
	U32 res;
	U8* pData=gUspiChannelDesc.Packet;
	//Add header
	*(U32*)pData=event;
	pData+=gUspiChannelDesc.hsize;
	memcpy(pData,data,gUspiChannelDesc.eventsize);
	res=usb_write(gUspiChannelDesc.pEpDesc,(const U8*)gUspiChannelDesc.Packet,gUspiChannelDesc.hsize+gUspiChannelDesc.eventsize ,0);
	if(!res)
	{
		//usb overflow
		gUspiChannelDesc.UsbOvflw++;
	}else
		gUspiChannelDesc.PktSent++;
}
