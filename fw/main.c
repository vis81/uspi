#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include <stdio.h>
#include <wchar.h>
#include "type.h"
#include "usb.h"
#include "spi.h"
#include "serial.h"
#include "chanmgr.h"
#include "timer.h"
#include "trace.h"
#include "pio.h"
#include "adc.h"
#include "intc.h"


const USB_DEVICE_DESCRIPTOR UsbDevDesc={
	sizeof(USB_DEVICE_DESCRIPTOR),	/* bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	0x0200,							/* bcdUSB */
	0x00,							/* bDeviceClass */
	0x00,							/* bDeviceSubClass */
	0x00,							/* bDeviceProtocol */
	USB_MAX_PACKET0,				/* bMaxPacketSize0 */
	VID,							/* idVendor */
	PID,							/* idProduct */
	0x0100,							/* bcdDevice */
	1,								/* iManufacturer */
	2,								/* iProduct */
	3,								/* iSerialNumber */
	1 								/* bNumConfigurations */
};

/// Configuration descriptors with one interface.
struct USB_CONFIG_DESCRIPTORS {

    USB_CONFIGURATION_DESCRIPTOR configuration;
    USB_INTERFACE_DESCRIPTOR 	interface;
	USB_ENDPOINT_DESCRIPTOR		endpoints[USB_EP_NUM-1];
};

/// Configuration descriptors.
const struct USB_CONFIG_DESCRIPTORS UsbCfgDesc = {

    // Configuration descriptor
    {
		sizeof(USB_CONFIGURATION_DESCRIPTOR),/* bLength */
		USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType */
		sizeof(struct USB_CONFIG_DESCRIPTORS),		/* wTotalLength */
		0x01,							   /* bNumInterfaces */
		0x01,							   /* bConfigurationValue */
		0x00,							   /* iConfiguration */
		USB_CONFIG_BUS_POWERED, 				/* bmAttributes */
		USB_CONFIG_POWER_MA(100),          /* bMaxPower */
		
    },
    // Interface descriptor
    {
        
		sizeof(USB_INTERFACE_DESCRIPTOR),	/* bLength */
		USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType */
		0x00,							   /* bInterfaceNumber */
		0x00,							   /* bAlternateSetting */
		USB_EP_NUM-1,							   /* bNumEndpoints */
		0xFF,								  /* bInterfaceClass */
		0xFF,								  /* bInterfaceSubClass */
		0xFF,							  /* bInterfaceProtocol */
		0								  /* iInterface */        
    },
    //endpoints
	{
		{
			sizeof(USB_ENDPOINT_DESCRIPTOR), 	/* bLength */
			USB_ENDPOINT_DESCRIPTOR_TYPE,	   /* bDescriptorType */
			USB_ENDPOINT_IN(1), 			   /* bEndpointAddress */
			USB_ENDPOINT_TYPE_BULK, 		   /* bmAttributes */
			0x0040,					   			/* wMaxPacketSize */
			0								   /* bInterval */
		},
	},
};

const USB_STRING_DESCRIPTOR str_lang_table={4,USB_STRING_DESCRIPTOR_TYPE,0x0409/* English */};
const USB_STRING_DESCRIPTOR sProduct={2+sizeof(PRODUCT),USB_STRING_DESCRIPTOR_TYPE,PRODUCT};
const USB_STRING_DESCRIPTOR sManufacturer={2+sizeof(MANUFACTURER),USB_STRING_DESCRIPTOR_TYPE,MANUFACTURER};
const USB_STRING_DESCRIPTOR sVersion={2+sizeof(VERSION),USB_STRING_DESCRIPTOR_TYPE,VERSION};

const void* strings[]={&str_lang_table,&sManufacturer,&sProduct,&sVersion,0};

S32 calibrate=-1;
U32 freemips=0;


U32 cmd_dispatch(USB_SETUP_PACKET* pSetup);



int main (void) {
  unsigned idle,lasttime=0;
  
  intc_init();
  
  uart_init();
  
  adc_init();
  
  usb_init (&UsbDevDesc,
  			(const USB_CONFIGURATION_DESCRIPTOR*)&UsbCfgDesc,
  			(const USB_STRING_DESCRIPTOR**)strings,
  			cmd_dispatch);

  idle=0;
  
  tm_start_rt();
  
  while(realtime<5);
  	lasttime=realtime;
	
  /* Loop forever */
  while (1) {
  	BOOL ret;
	ret=adc_task();
	ret|=tm_task();
  	ret|=usb_task();
		
	if(!ret)
		idle++;
	if(realtime>lasttime)
	{
		if(calibrate==-1)
		{
			calibrate=idle;
			TRACE_INFO("Calibrate loop %u\n",calibrate);
		}
		freemips=idle*1000/calibrate;
		lasttime=realtime;
		idle=0;
	}
  }
}

