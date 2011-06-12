#ifndef _USB_H_
#define _USB_H_
#include "config.h"
#include "AT91SAM7S64.H"                    /* AT91SAM7S64 definitions */
#include "type.h"


/*************************************************************************
				DEFINES
**************************************************************************/


#define USB_MAX_PACKET0     8
#define USB_MAX_PACKET      64
#define EVENTQ_SIZE			10


#define NO_ZLP   0
#define NEED_ZLP 1
#define ZLP_SENT 2


/* USB Endpoint Callback Events */
#define USB_EVT_SETUP       1   /* Setup Packet */

/* bmRequestType.Dir */
#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

/* bmRequestType.Type */
#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

/* bmRequestType.Recipient */
#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

/* USB Standard Request Codes */
#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

/* USB GET_STATUS Bit Values */
#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

/* USB Standard Feature selectors */
#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1


/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    8


/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0xC0
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0x40
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/// Bitmap for all status bits in CSR.
#define REG_NO_EFFECT_1_ALL      AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1 \
                                |AT91C_UDP_STALLSENT   | AT91C_UDP_RXSETUP \
                                |AT91C_UDP_TXCOMP





/*************************************************************************
				MACROS
**************************************************************************/

#if 0
/// Clears the specified bit(s) in the UDP_CSR register.
/// \param endpoint The endpoint number of the CSR to process.
/// \param flags The bitmap to set to 1.
#define SET_CSR(csr, flags) \
    { \
        volatile unsigned int reg; \
        reg = *csr ; \
        reg |= REG_NO_EFFECT_1_ALL; \
        reg |= (flags); \
        *csr = reg; \
        while ( (*csr & (flags)) != (flags)); \
    }

/// Sets the specified bit(s) in the UDP_CSR register.
/// \param endpoint The endpoint number of the CSR to process.
/// \param flags The bitmap to clear to 0.
#define CLEAR_CSR(csr, flags) \
    { \
        volatile unsigned int reg; \
        reg = *csr; \
        reg |= REG_NO_EFFECT_1_ALL; \
        reg &= ~(flags); \
        *csr = reg; \
        while ( (*csr & (flags)) == (flags)); \
    }

#else

#define SET_CSR(csr, flags) *(csr) |= (flags)
#define CLEAR_CSR(csr, flags) *(csr) &= ~(flags)



#endif

#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))





/*************************************************************************
				STRUCTURES
**************************************************************************/

typedef __packed union {
  U16 W;
  __packed struct {
    U8 L;
    U8 H;
  } __GCC_PACKED__ WB;
} __GCC_PACKED__ WORD_BYTE;

/* bmRequestType Definition */
typedef __packed union _REQUEST_TYPE {
  __packed struct _BM {
    U8 Recipient : 5;
    U8 Type      : 2;
    U8 Dir       : 1;
  } __GCC_PACKED__ BM;
  U8 B;
} __GCC_PACKED__ REQUEST_TYPE;

/* USB Default Control Pipe Setup Packet */
typedef __packed struct _USB_SETUP_PACKET {
  REQUEST_TYPE bmRequestType;
  U8         bRequest;
  WORD_BYTE    wValue;
  WORD_BYTE    wIndex;
  U16         wLength;
  U8			Payload[MAX_EP0_PAYLOAD_SIZE];
} __GCC_PACKED__ USB_SETUP_PACKET;


/* USB Standard Device Descriptor */
typedef __packed struct _USB_DEVICE_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bcdUSB;
  U8  bDeviceClass;
  U8  bDeviceSubClass;
  U8  bDeviceProtocol;
  U8  bMaxPacketSize0;
  U16  idVendor;
  U16  idProduct;
  U16  bcdDevice;
  U8  iManufacturer;
  U8  iProduct;
  U8  iSerialNumber;
  U8  bNumConfigurations;
} __GCC_PACKED__ USB_DEVICE_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
typedef __packed struct _USB_CONFIGURATION_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  wTotalLength;
  U8  bNumInterfaces;
  U8  bConfigurationValue;
  U8  iConfiguration;
  U8  bmAttributes;
  U8  MaxPower;
} __GCC_PACKED__ USB_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor */
typedef __packed struct _USB_INTERFACE_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U8  bInterfaceNumber;
  U8  bAlternateSetting;
  U8  bNumEndpoints;
  U8  bInterfaceClass;
  U8  bInterfaceSubClass;
  U8  bInterfaceProtocol;
  U8  iInterface;
} __GCC_PACKED__ USB_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor */
typedef __packed struct _USB_ENDPOINT_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U8  bEndpointAddress;
  U8  bmAttributes;
  U16  wMaxPacketSize;
  U8  bInterval;
} __GCC_PACKED__ USB_ENDPOINT_DESCRIPTOR;


/* USB String Descriptor */
typedef __packed struct _USB_STRING_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bString[20];
} __GCC_PACKED__ USB_STRING_DESCRIPTOR;

/* USB Common Descriptor */
typedef __packed struct _USB_COMMON_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
} __GCC_PACKED__ USB_COMMON_DESCRIPTOR;


typedef U32 (*EP_WriteCB) (HANDLE h);
typedef U32 (*EP_SetupCB)(USB_SETUP_PACKET* pSetup);

	

typedef struct _EP_DESC {
	struct _USB_DESC* pUsbDevice;
	U8	EpId;
	U8	Direction;
	U8	FifoSize;
	BOOL    Flush;
	AT91_REG* UDP_CSR;
	AT91_REG* UDP_FDR;

	U16	WBufSize;
	U8*	pWBuffer;
	U8*	pWBufEnd;
	U8*	pWData;
	U8*	pWPutData;
	U16		BytesLeft;
	U16 	BytesReady;
	U16 	BytesSent;
	U32 	BytesDone;


	U8*	pRData;
	U16		RBytesLeft;

	
	U16		tEVT;
	U8	ZLP;
	
	USB_SETUP_PACKET* pSetupPacket;//CTRL only

	U32 TxIRQ;
} EP_DESC;


typedef struct _USB_DESC {
	AT91PS_UDP pUDP;
	const USB_DEVICE_DESCRIPTOR* pUSB_DeviceDescriptor;
	const USB_CONFIGURATION_DESCRIPTOR* pUSB_ConfigDescriptor;
	const USB_STRING_DESCRIPTOR** pUSB_SringsDescriptors;
	U8		NumString;
	U8  USB_DeviceAddress;
	USB_SETUP_PACKET SetupPacket;
	EP_DESC EpDesc[USB_EP_NUM];
	EP_SetupCB SetupCB;
	U32 IRQ;
} USB_DESC;






/*************************************************************************
				PUBLIC FUNCTIONS
**************************************************************************/

void usb_init (const USB_DEVICE_DESCRIPTOR* pDevDesc,
				const USB_CONFIGURATION_DESCRIPTOR* pCfgDesc,
				const USB_STRING_DESCRIPTOR** pSrings,
				EP_SetupCB cb
				);
void usb_connect (BOOL con);
BOOL usb_task(void);
EP_DESC* usb_open(U8 EpId);
#ifdef __GNUC__
static inline U32 usb_reset_ep(EP_DESC* pEpDesc)
{
	U32 ret;
	asm volatile (
		"mov r0, %1 \n\t" \
		"swi 2    \n\t" \
		"mov %0, r0 \n\t" \
		: "=r" (ret)
		: "r"  (pEpDesc)
		:"r0", "r1", "r2", "r3", "memory", "cc"
	);
	return ret;
}

static inline void usb_flush(EP_DESC* pEpDesc)
{
	asm volatile (
		"mov r0, %0 \n\t" \
		"swi 3    \n\t" \
		: : "r" (pEpDesc)
		:"r0", "r1", "r2", "r3", "memory", "cc"
	);
}

static inline U32 usb_write(EP_DESC* pEpDesc,const U8* pData,U32 cnt,U16 evt)
{
	U32 ret;
	asm volatile (
		"mov r0, %1 \n\t" \
		"mov r1, %2 \n\t" \
		"mov r2, %3 \n\t" \
		"mov r3, %4 \n\t" \
		"swi 1    \n\t" \
		"mov %0, r0 \n\t" \
		: "=r" (ret)
		: "r"  (pEpDesc), "r" (pData), "r" (cnt), "r" (evt) 
		:"r0", "r1", "r2", "r3","memory", "cc"
	);
	return ret;
}

#else
U32 __swi(1) usb_write(EP_DESC* pEpDesc,const U8* pData,U32 cnt,U16 evt);
U32 __swi(2) usb_reset_ep (EP_DESC* pEpDesc);
void __swi(3) usb_flush(EP_DESC* pEpDesc);
#endif


#endif
