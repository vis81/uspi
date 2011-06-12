#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include <stdio.h>
#include "type.h"
#include "usb.h"
#include "chanmgr.h"
#include "timer.h"
#include "stdlib.h"
#include "serial.h"
#include "config.h"
#include "string.h"
#include "trace.h"
#include "assert.h"
#include "intc.h"

/*************************************************************************
				PRIVATE VARS
**************************************************************************/
USB_DESC gUsbDesc;
static U8 BulkEpBuf[USB_EP_NUM-1][USB_EP_BUFSIZE];
static U8 CtrlEpBuf[USB_EP0_BUFSIZE];
volatile BOOL gUsbEvent;

#if CALC_USB_LATENCY
U32 usb_time;
U32 usb_maxtime;
#endif


/*************************************************************************
				PRIVATE FUNCTIONS DECLARATION
**************************************************************************/
void usbisr_get_setuppkt (EP_DESC* pEpDesc);
U32 usbisr_write_ep(EP_DESC* pEpDesc,const U8* pData,U32 cnt,U16 evt);
void usbisr_flush_ep(EP_DESC* pEpDesc);
U32 usbisr_read_ep (EP_DESC* pEpDesc,U8* pData,U32 cnt,U16 evt);
void usbisr_enable_ep (EP_DESC* pEpDesc,BOOL Enable);
void usbisr_stall_ep (EP_DESC* pEpDesc,BOOL Stall);
U32 usbisr_reset_ep (EP_DESC* pEpDesc);
void usbisr_config_ep (const USB_ENDPOINT_DESCRIPTOR *pEPD);
BOOL usbisr_getstatus_ep (EP_DESC* pEpDesc);


BOOL usbisr_setcfg (U8 Configuration);
void usbisr_reset (void);
void usbisr_writefifo (EP_DESC* pEpDesc);
U32 usbisr_readreq (EP_DESC* pEpDesc);
U32 usbisr_writereq (EP_DESC* pEpDesc);
void usbisr_setupreq (EP_DESC* pEpDesc);
void usb_isr (void);


/*************************************************************************
				FUNCTION DEFINITIONS
**************************************************************************/
const USB_COMMON_DESCRIPTOR* usb_get_desc (U8 Recipient,U8 Type,U8 Index,U32* retlen) {
	const USB_COMMON_DESCRIPTOR* pD=NULL;
	U32 len=0;
	if(Recipient==REQUEST_TO_DEVICE) 
	{
		switch (Type) 
		{
			case USB_DEVICE_DESCRIPTOR_TYPE:
				pD=(const USB_COMMON_DESCRIPTOR*)gUsbDesc.pUSB_DeviceDescriptor;
				len=gUsbDesc.pUSB_DeviceDescriptor->bLength;
				break;
				
			case USB_CONFIGURATION_DESCRIPTOR_TYPE:
				if(Index==0) 
				{
					pD = (const USB_COMMON_DESCRIPTOR *)gUsbDesc.pUSB_ConfigDescriptor;
					len = gUsbDesc.pUSB_ConfigDescriptor->wTotalLength;
				}
				break;
			case USB_STRING_DESCRIPTOR_TYPE:
				if(Index<gUsbDesc.NumString)
				{
					pD=(const USB_COMMON_DESCRIPTOR*)gUsbDesc.pUSB_SringsDescriptors[Index];
					len = pD->bLength;
				}
				break;
		}
	}
	*retlen=len;
	return pD;
}


void usb_init (const USB_DEVICE_DESCRIPTOR* pDevDesc,
				const USB_CONFIGURATION_DESCRIPTOR* pCfgDesc,
				const USB_STRING_DESCRIPTOR** pSrings,
				EP_SetupCB cb
				)

{
	U32 i;
	/* Enables the 48MHz USB Clock UDPCK and System Peripheral USB Clock */
	AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

	intc_connect(AT91C_ID_UDP,
		AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST,
		usb_isr, FALSE);

	gUsbDesc.pUDP=AT91C_BASE_UDP;

	for(i=0;i<USB_EP_NUM;i++)
	{
		gUsbDesc.EpDesc[i].EpId=i;
		gUsbDesc.EpDesc[i].pUsbDevice=&gUsbDesc;
		
		gUsbDesc.EpDesc[i].UDP_CSR=&gUsbDesc.pUDP->UDP_CSR[i];
		gUsbDesc.EpDesc[i].UDP_FDR=&gUsbDesc.pUDP->UDP_FDR[i];

		usb_reset_ep(&gUsbDesc.EpDesc[i]);
		if(i>0){
			gUsbDesc.EpDesc[i].FifoSize=USB_MAX_PACKET;
			gUsbDesc.EpDesc[i].pWBuffer=&BulkEpBuf[i-1][0];
			gUsbDesc.EpDesc[i].WBufSize=USB_EP_BUFSIZE;
		}
		else
		{
			
			gUsbDesc.EpDesc[i].pWBuffer=CtrlEpBuf;
			gUsbDesc.EpDesc[i].WBufSize=USB_EP0_BUFSIZE;
			gUsbDesc.EpDesc[i].FifoSize=USB_MAX_PACKET0;
			gUsbDesc.EpDesc[i].pSetupPacket=&gUsbDesc.SetupPacket;
		}
		gUsbDesc.EpDesc[i].pWBufEnd=gUsbDesc.EpDesc[i].pWBuffer+gUsbDesc.EpDesc[i].WBufSize;
	}
	
	gUsbDesc.pUSB_DeviceDescriptor=pDevDesc;
	gUsbDesc.pUSB_ConfigDescriptor=pCfgDesc;
	gUsbDesc.pUSB_SringsDescriptors=pSrings;
	gUsbDesc.NumString=0;
	while(pSrings[gUsbDesc.NumString])
		gUsbDesc.NumString++;

	gUsbDesc.SetupCB=cb;

	TRACE_INFO("USB: init done VID=%x PID=%x\n",pDevDesc->idVendor,pDevDesc->idProduct);
}

void usb_connect (BOOL con) {
	return;
}

void usbisr_reset (void) {
  U32 i;

  /* Global USB Reset */
  gUsbDesc.pUDP->UDP_GLBSTATE &= ~(AT91C_UDP_FADDEN | AT91C_UDP_CONFG | AT91C_UDP_RMWUPE);
  gUsbDesc.pUDP->UDP_FADDR     =  AT91C_UDP_FEN;
  gUsbDesc.pUDP->UDP_ICR       =  0xFFFFFFFF;

  /* Reset & Disable USB Endpoints */
  for(i=0;i<USB_EP_NUM;i++)
	  usbisr_reset_ep(&gUsbDesc.EpDesc[i]);
  
  gUsbDesc.pUDP->UDP_RSTEP = 0xFFFFFFFF;
  gUsbDesc.pUDP->UDP_RSTEP = 0;

  /* Setup USB Interrupts */
  gUsbDesc.pUDP->UDP_IER = AT91C_UDP_RXSUSP | AT91C_UDP_RXRSM | AT91C_UDP_EXTRSM | 
                  ((1<<USB_EP_NUM)-1);


  /* Setup Control Endpoint 0 */
  gUsbDesc.pUDP->UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;
  
  gUsbDesc.USB_DeviceAddress = 0;  
  TRACE_INFO("USB: Reset\n");
}

BOOL usbisr_setcfg (U8 Configuration) {
	const USB_CONFIGURATION_DESCRIPTOR *pConfig;
	U32 n;
	pConfig=gUsbDesc.pUSB_ConfigDescriptor;
	if (Configuration==pConfig->bConfigurationValue)
	{
		const USB_INTERFACE_DESCRIPTOR* pInterface;
		const USB_ENDPOINT_DESCRIPTOR*  pEndpoint;
		U8 EpCnt;
		
		//Disable all EPs except 0
		for (n = 1; n < USB_EP_NUM; n++)
			usbisr_enable_ep(&gUsbDesc.EpDesc[n],FALSE);
		
		gUsbDesc.pUDP->UDP_GLBSTATE |=  AT91C_UDP_CONFG;

		pInterface=(const USB_INTERFACE_DESCRIPTOR*)((U8*)pConfig+pConfig->bLength);
		EpCnt=pInterface->bNumEndpoints;
		pEndpoint=(const USB_ENDPOINT_DESCRIPTOR*)((U8*)pInterface+pInterface->bLength);
		while(EpCnt--)
		{
			U8 EpId=pEndpoint->bEndpointAddress&0xF;
			usbisr_config_ep(pEndpoint);
			usbisr_enable_ep(&gUsbDesc.EpDesc[EpId],TRUE);
			usbisr_reset_ep(&gUsbDesc.EpDesc[EpId]);
			pEndpoint=(const USB_ENDPOINT_DESCRIPTOR*)((U8*)pEndpoint+pEndpoint->bLength);
		}
		return TRUE;
	}
	
	if (Configuration==0)
	{
		for (n = 1; n < USB_EP_NUM; n++)
			usbisr_enable_ep(&gUsbDesc.EpDesc[n],FALSE);
			gUsbDesc.pUDP->UDP_GLBSTATE &= ~AT91C_UDP_CONFG;
	}
	return (FALSE);
}

BOOL usbisr_getstatus_ep (EP_DESC* pEpDesc) {
	//2 TODO
	return FALSE;
}

void usbisr_config_ep (const USB_ENDPOINT_DESCRIPTOR *pEPD) {
  U32 num, dir,type;
  num = pEPD->bEndpointAddress & 0x0F;
  dir = (pEPD->bEndpointAddress & USB_ENDPOINT_DIRECTION_MASK)<<3;
  type=pEPD->bmAttributes & USB_ENDPOINT_TYPE_MASK;  
  gUsbDesc.pUDP->UDP_CSR[num]=(type<<8)|dir;
}


void usbisr_setupreq (EP_DESC* pEpDesc) 
{
	U8 dir;
	USB_SETUP_PACKET* pSetupPacket;
	if(pEpDesc->EpId!=0){
		TRACE_FATAL("USB: Setup %u\n",pEpDesc->EpId);
	}
		
	usbisr_get_setuppkt(pEpDesc);
	
	CLEAR_CSR(pEpDesc->UDP_CSR,AT91C_UDP_RXSETUP);
	
	pSetupPacket=pEpDesc->pSetupPacket;
	
	dir=pSetupPacket->bmRequestType.BM.Dir;
	if (dir==REQUEST_DEVICE_TO_HOST ) {
		*pEpDesc->UDP_CSR |=  AT91C_UDP_DIR;
	}

	switch (pSetupPacket->bmRequestType.BM.Type) {
		case REQUEST_STANDARD:
			switch (pSetupPacket->bRequest) 
			{
				case USB_REQUEST_GET_STATUS:
					if (!usbisr_getstatus_ep(pEpDesc))
						goto stall_i;
					break;
				
				case USB_REQUEST_SET_ADDRESS:
					gUsbDesc.USB_DeviceAddress = pSetupPacket->wValue.WB.L;
					SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
					while(!(*pEpDesc->UDP_CSR&AT91C_UDP_TXCOMP));
					CLEAR_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXCOMP);
					gUsbDesc.pUDP->UDP_FADDR = AT91C_UDP_FEN | gUsbDesc.USB_DeviceAddress;
					gUsbDesc.pUDP->UDP_GLBSTATE |=  AT91C_UDP_FADDEN;
					TRACE_DEBUG("USB: Set addr %x\n",gUsbDesc.USB_DeviceAddress);
					break;
				case USB_REQUEST_GET_DESCRIPTOR:
					{
						const USB_COMMON_DESCRIPTOR* pD;
						U32 len;
						TRACE_DEBUG("USB: Get desc %u\n",pSetupPacket->wValue.WB.H);
						pD=usb_get_desc(
							pSetupPacket->bmRequestType.BM.Recipient,
							pSetupPacket->wValue.WB.H,
							pSetupPacket->wValue.WB.L,
							&len
							);
						if(!pD)
							goto stall_i;
						len=MIN(len,pSetupPacket->wLength);
						usbisr_write_ep(pEpDesc,(const U8*)pD,len,0);
						break;
					}

				case USB_REQUEST_SET_CONFIGURATION:
					TRACE_DEBUG("USB: Set config %x\n",pSetupPacket->wValue.WB.L);
					if (!usbisr_setcfg(pSetupPacket->wValue.WB.L))
						goto stall_i;
					SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
					break;

				case USB_REQUEST_CLEAR_FEATURE:
					usbisr_reset_ep(&gUsbDesc.EpDesc[pSetupPacket->wIndex.W&0x3]);
					SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
					break;
				default:
				goto stall_i;
			}
		break;

		case REQUEST_VENDOR:
			TRACE_DEBUG("USB: Vendor %x\n",pEpDesc->pSetupPacket->bRequest);
			if(dir==REQUEST_HOST_TO_DEVICE)
				if(pEpDesc->pSetupPacket->wLength<=MAX_EP0_PAYLOAD_SIZE)
				{
					if(pEpDesc->pSetupPacket->wLength)
					{
						usbisr_read_ep(pEpDesc,pEpDesc->pSetupPacket->Payload,
							pEpDesc->pSetupPacket->wLength,USB_EVT_SETUP);
						return;
					}
				}else
				{
					usbisr_stall_ep (pEpDesc,TRUE);
					return;
				}
			SANITY_CHECK(!gUsbEvent);
			gUsbEvent=TRUE;
			break;
		default:
stall_i:
		usbisr_stall_ep(pEpDesc,TRUE);
		break;
	}
}

U32 usbisr_readreq (EP_DESC* pEpDesc) {
	U32 cnt;
	if(!pEpDesc->pRData)
	{
		if(pEpDesc->ZLP==NEED_ZLP)//RX ZLP done
			pEpDesc->ZLP=NO_ZLP;
		else
			usbisr_stall_ep(pEpDesc,TRUE);
		return 0;
	}
	cnt = (*pEpDesc->UDP_CSR&AT91C_UDP_RXBYTECNT) >> 16;
	while(cnt)
	{
		*pEpDesc->pRData++=*pEpDesc->UDP_FDR;
		pEpDesc->RBytesLeft--;
		cnt--;
		if(!pEpDesc->RBytesLeft)
		{
			if(pEpDesc->tEVT)
			{
				pEpDesc->tEVT=0;
				if(gUsbEvent)
					while(1);
				else
					gUsbEvent=TRUE;
			}
			pEpDesc->pRData=NULL;
			break;
		}
	}
	return cnt;
}

U32 usbisr_writereq(EP_DESC* pEpDesc) {
	pEpDesc->BytesDone+=pEpDesc->BytesSent;
	pEpDesc->BytesSent=0;
	usbisr_writefifo (pEpDesc);
	
	if(!pEpDesc->BytesLeft && pEpDesc->EpId==0)
		pEpDesc->Flush=TRUE;
	
	if(pEpDesc->BytesReady==pEpDesc->FifoSize || pEpDesc->Flush)
	{
		pEpDesc->BytesSent=pEpDesc->BytesReady;
		pEpDesc->BytesReady=0;
		SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
		if(pEpDesc->BytesSent<pEpDesc->FifoSize)
			pEpDesc->Flush=FALSE;
	}
	return 0;
}

U32 usbisr_read_ep (EP_DESC* pEpDesc,U8* pData,U32 cnt,U16 evt) {
	pEpDesc->pRData=pData;
	pEpDesc->RBytesLeft=cnt;
	pEpDesc->tEVT=evt;
	return cnt;
}


void usbisr_get_setuppkt (EP_DESC* pEpDesc) 
{
	U8 *pData=(U8*)pEpDesc->pSetupPacket;
	U8 cnt;
	for(cnt=0;cnt<8;cnt++)
		*pData++=*pEpDesc->UDP_FDR;
}


void usbisr_flush_ep(EP_DESC* pEpDesc)
{
	pEpDesc->Flush=TRUE;
	usbisr_writefifo(pEpDesc);
	if(!pEpDesc->BytesSent)
	{
		pEpDesc->BytesSent=pEpDesc->BytesReady;
		pEpDesc->BytesReady=0;
		SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
		if(pEpDesc->BytesSent<pEpDesc->FifoSize)
			pEpDesc->Flush=FALSE;
	}
		
}

U32 usbisr_write_ep(EP_DESC* pEpDesc,const U8* pData,U32 cnt,U16 evt)
{
	unsigned ret;
	U8* pend;
	U8* pput;
	
	ret=MIN(cnt,pEpDesc->WBufSize-pEpDesc->BytesLeft);	
	if(ret!=cnt){		
		return 0;
	}	
	ret=cnt;
	
	pEpDesc->BytesLeft+=cnt;

	pput=pEpDesc->pWPutData;
	pend=pEpDesc->pWBufEnd;
	while(cnt--)
	{
		*pput++=*pData++;
		if(pput==pend)
			pput=pEpDesc->pWBuffer;
	}
	pEpDesc->pWPutData=pput;

	if(pEpDesc->EpId==0)
		pEpDesc->ZLP=NEED_ZLP;
	
	if(!pEpDesc->BytesSent)
		usbisr_writereq(pEpDesc);
	return ret;
}

void usbisr_writefifo (EP_DESC* pEpDesc) 
{
	U32 cnt;
	cnt=MIN(pEpDesc->FifoSize-pEpDesc->BytesReady,
			pEpDesc->BytesLeft);
	if(cnt)
	{
	
		AT91_REG* pfdr=pEpDesc->UDP_FDR;
		U8* pend=pEpDesc->pWBufEnd;		
		U8* pstart=pEpDesc->pWBuffer;
		U8* pWData=pEpDesc->pWData;
		pEpDesc->BytesReady+=cnt;
		pEpDesc->BytesLeft-=cnt;

		
		while(cnt--)
		{
			*pfdr=*pWData++;
			if(pWData==pend)
				pWData=pstart;
		}		

		pEpDesc->pWData=pWData;
	}
}


void usbisr_stall_ep (EP_DESC* pEpDesc, BOOL Stall)
{
   if(Stall){
	   	TRACE_WARNING("USB: Stall %u\n",pEpDesc->EpId);
		*pEpDesc->UDP_CSR|=AT91C_UDP_FORCESTALL;
   	}
   else
		*pEpDesc->UDP_CSR&=~AT91C_UDP_FORCESTALL;
}

void usbisr_enable_ep (EP_DESC* pEpDesc,BOOL Enable) {
	if(Enable)
		*pEpDesc->UDP_CSR |=  AT91C_UDP_EPEDS;
	else
		*pEpDesc->UDP_CSR &= ~AT91C_UDP_EPEDS;
}

U32 usbisr_reset_ep (EP_DESC* pEpDesc)
{

	if(*pEpDesc->UDP_CSR&AT91C_UDP_TXPKTRDY)
	{
		CLEAR_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
		while(*pEpDesc->UDP_CSR&AT91C_UDP_TXPKTRDY);
		SET_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
		while(!(*pEpDesc->UDP_CSR&AT91C_UDP_TXPKTRDY));
		CLEAR_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXPKTRDY);
		while(*pEpDesc->UDP_CSR&AT91C_UDP_TXPKTRDY);
	}
	gUsbDesc.pUDP->UDP_RSTEP  |=   (1 << pEpDesc->EpId);
	gUsbDesc.pUDP->UDP_RSTEP  &= ~(1 << pEpDesc->EpId);
		
	pEpDesc->pWData=pEpDesc->pWBuffer;
	pEpDesc->pWPutData=pEpDesc->pWBuffer;
	pEpDesc->BytesLeft=0;
	pEpDesc->BytesReady=0;
	pEpDesc->BytesDone=0;
	pEpDesc->BytesSent=0;
	pEpDesc->TxIRQ=0;

	pEpDesc->pRData=NULL;
	pEpDesc->RBytesLeft=0;

	pEpDesc->Flush=FALSE;
	return 0;
}


/*
 *  USB Interrupt Service Routine
 */
void usb_isr (void)
{
	U32 isr, csr, n;  
	static AT91PS_UDP pUDP = AT91C_BASE_UDP;
	static AT91_REG* UDP_ICR=&AT91C_BASE_UDP->UDP_ICR;
	static AT91_REG* UDP_CSR[4]={	&AT91C_BASE_UDP->UDP_CSR[0],
						&AT91C_BASE_UDP->UDP_CSR[1],
						&AT91C_BASE_UDP->UDP_CSR[2],
						&AT91C_BASE_UDP->UDP_CSR[3]
						};
	
	gUsbDesc.IRQ++;
	isr=pUDP->UDP_ISR;
	{
		/* End of Bus Reset Interrupt */
		if (isr & AT91C_UDP_ENDBUSRES) {
			usbisr_reset();
			*UDP_ICR = AT91C_UDP_ENDBUSRES;
		}
		/* USB Suspend Interrupt */
		if (isr & AT91C_UDP_RXSUSP) {
			*UDP_ICR = AT91C_UDP_RXSUSP;
		}
		/* USB Resume Interrupt */
		if (isr & AT91C_UDP_RXRSM) {
			*UDP_ICR = AT91C_UDP_RXRSM;
		}
		/* External Resume Interrupt */
		if (isr & AT91C_UDP_EXTRSM) {
			*UDP_ICR = AT91C_UDP_EXTRSM;
		}
		/* Start of Frame Interrupt */
		if (isr & AT91C_UDP_SOFINT) {
			*UDP_ICR = AT91C_UDP_SOFINT;
		}

		/* Endpoint Interrupts */
		for (n = 0; n < USB_EP_NUM; n++) 
		{
			if (isr & (1 << n)) 
			{
				EP_DESC* pEpDesc=&gUsbDesc.EpDesc[n];
				csr = *UDP_CSR[n];

				/* Setup Packet Received Interrupt */
				if (csr & AT91C_UDP_RXSETUP) 
				{
					usbisr_setupreq(pEpDesc);
				}
				
				/* Data Packet Received Interrupt */
				if (csr & AT91C_UDP_RX_DATA_BK0) 
				{
					usbisr_readreq(pEpDesc);
					*UDP_CSR[n] &= ~AT91C_UDP_RX_DATA_BK0;
				}

				/* Data Packet Sent Interrupt */
				if (csr & AT91C_UDP_TXCOMP) 
				{
#if CALC_USB_LATENCY
				
					if(usb_time>usb_maxtime)
					{
						usb_maxtime=usb_time;
						TRACE_INFO("USB latency=%u\n",usb_maxtime);
					}
					usb_time=0;
#endif
					pEpDesc->TxIRQ++;
					usbisr_writereq(pEpDesc);
					CLEAR_CSR(pEpDesc->UDP_CSR,AT91C_UDP_TXCOMP);
					
				}
				/* STALL Packet Sent Interrupt */
				if (csr & AT91C_UDP_STALLSENT) 
				{
					if ((csr & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL) 
						usbisr_stall_ep (pEpDesc,FALSE);
					*UDP_CSR[n] &= ~AT91C_UDP_STALLSENT;
				}
				*UDP_ICR = 1 << n;
			}
		}
	}
	
}


EP_DESC* usb_open(U8 EpId)
{
	if(EpId<USB_EP_NUM)
		return &gUsbDesc.EpDesc[EpId];
	return NULL;
}

BOOL usb_task()
{
	BOOL busy=FALSE;
	U32 ret;
	if(gUsbEvent)
	{
		EP_DESC* pEpDesc=&gUsbDesc.EpDesc[0];
		USB_SETUP_PACKET* pSetupPacket=pEpDesc->pSetupPacket;
		busy=TRUE;
		ret=gUsbDesc.SetupCB(pSetupPacket);
		
		if (pSetupPacket->bmRequestType.BM.Dir == REQUEST_DEVICE_TO_HOST)
			usb_write(pEpDesc,(U8*)pSetupPacket->Payload,ret,0);
		else
			usb_flush(pEpDesc);
		gUsbEvent=FALSE;
	}
	return busy;
}

