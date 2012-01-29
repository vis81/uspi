#include "type.h"
#include "commands.h"
#include "timer.h"
#include "usb.h"
#include "chanmgr.h"
#include "string.h"
#include "spi.h"
#include "adc.h"
#include "twi.h"


typedef U32 (*cmd_cb)(USB_SETUP_PACKET* pSetup);

extern U32 freemips;




U32 cmd_getmips(USB_SETUP_PACKET* pSetup);
U32 cmd_start(USB_SETUP_PACKET* pSetup);
U32 cmd_stop(USB_SETUP_PACKET* pSetup);
U32 cmd_getstat(USB_SETUP_PACKET* pSetup);
U32 cmd_setspi(USB_SETUP_PACKET* pSetup);
U32 cmd_i2c(USB_SETUP_PACKET* pSetup);
U32 cmd_i2c_speed(USB_SETUP_PACKET* pSetup);




const cmd_cb commands[NUM_CMDS]={cmd_getmips,cmd_start,cmd_stop,cmd_getstat,cmd_setspi,cmd_i2c,cmd_i2c_speed};


U32 cmd_dispatch(USB_SETUP_PACKET* pSetup)
{
	if(pSetup->bRequest>=NUM_CMDS)
		return 0;
	return commands[pSetup->bRequest](pSetup);
}

U32 cmd_getmips(USB_SETUP_PACKET* pSetup)
{
	memcpy(pSetup->Payload,&freemips,4);
	return 4;
}

U32 cmd_start(USB_SETUP_PACKET* pSetup)
{
	U8 	spi=0,
		drdy=0,
		hsize=4,
		adcnum=3;

	if(pSetup->wLength>0)
		spi=pSetup->Payload[0];

	if(pSetup->wLength>1)
		drdy=pSetup->Payload[1];

	if(pSetup->wLength>2)
		adcnum=pSetup->Payload[2];
	
	if(pSetup->wLength>3)
		hsize=pSetup->Payload[3];	

	if(adcnum>4 || adcnum<1)
		adcnum=3;

	if(hsize>4)
		hsize=4;
	
	if(drdy>3)
		drdy=0;
	
	if(spi>3)
		spi=0;
	
	chan_start(spi,drdy,adcnum,hsize);
	return 0;
}

U32 cmd_stop(USB_SETUP_PACKET* pSetup)
{
	chan_stop();
	return 0;
}

U32 cmd_getstat(USB_SETUP_PACKET* pSetup)
{
	struct cmd_getstat_out stat;

	stat.PktSent = gUspiChannelDesc.PktSent;
	stat.UsbOvflw = gUspiChannelDesc.UsbOvflw;
	stat.SpiOverRun = gAdcDesc.spi_ovrun;

	memcpy(pSetup->Payload,&stat,sizeof(stat));
	return sizeof(stat);
}

U32 cmd_setspi(USB_SETUP_PACKET* pSetup)
{
	spi_config( pSetup->Payload[0],
				pSetup->Payload[1]
			);
	return 0;
}


U32 cmd_i2c(USB_SETUP_PACKET* pSetup)
{
	struct cmd_i2c_out *out = (struct cmd_i2c_out*) &pSetup->Payload[0];
	unsigned char dadr;
	unsigned char iadr;
	unsigned char data;
	unsigned char bwrite;

	if(pSetup->wLength < 2){
		out->err=2;
		goto exit;
	}
	data = pSetup->wValue.WB.L;
	bwrite = pSetup->wValue.WB.H;
	dadr = pSetup->wIndex.WB.L;
	iadr = pSetup->wIndex.WB.H;
	

	if(bwrite)
		out->err=twi_writebyte(dadr,iadr,1,&data);
	else
		out->err=twi_readbyte(dadr,iadr,1,&out->data);
exit:			
	return 2;
}



U32 cmd_i2c_speed(USB_SETUP_PACKET* pSetup)
{
	unsigned int speed;
	if(pSetup->wLength < 4)
		return 0;
	memcpy(&speed,&pSetup->Payload[0],4);
	if(speed<10000 || speed>400000)
		return 0;
	twi_setclk(speed);
	return 0;
}

