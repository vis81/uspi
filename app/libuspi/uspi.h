#ifndef _USPI_H_
#define _USPI_H_
#include "stdio.h"

#define USPI_NUM_CHANNELS 3
#define USPI_BYTES_PER_SAMPLE 3

/*******************************************************************************************
******************************   TYPES     *************************************************
********************************************************************************************/
struct uspi_handle;
typedef struct uspi_handle uspi_handle;

struct uspi_stat {
	unsigned int PktSent;
	unsigned int UsbOvflw;
	unsigned int SpiOverRun;
    unsigned int BufLevel;
};

struct uspi_sample {
	unsigned int time;
	unsigned char data[USPI_NUM_CHANNELS][USPI_BYTES_PER_SAMPLE];
} __attribute__((__packed__));

/*******************************************************************************************
******************************   FUNCTIONS     *********************************************
********************************************************************************************/


/*******************************************************************************************
 FUNCTION:    uspi_open
 DESCRIPTION: open uspi device
 PARAMS:      none
 RETURNS:     handle to uspi device
********************************************************************************************/
uspi_handle* uspi_open();

int uspi_fw_version(uspi_handle* uspi,char* buf, unsigned size);


/*******************************************************************************************
 FUNCTION:    uspi_close
 DESCRIPTION: close uspi device
 PARAMS:      dev - handle to uspi device
 RETURNS:     void
********************************************************************************************/
void uspi_close(uspi_handle* dev);


/*******************************************************************************************
 FUNCTION:    uspi_getmips
 DESCRIPTION: get number of current free CPU MIPSes in % with 0.1% precision
 PARAMS:      dev - handle to uspi device
 RETURNS:     current free CPU MIPSes in 0.1% units
********************************************************************************************/
int uspi_getmips(uspi_handle* dev);


/*******************************************************************************************
 FUNCTION:    uspi_setspi
 DESCRIPTION: configure SPI
 PARAMS:        dev - handle to uspi device
                loopback - internal SPI loopback enable
                SCBR - SPI clock baud rate divisor 1..255 (0-forbidden)
 RETURNS:     >0 on success
********************************************************************************************/
int uspi_setspi(uspi_handle* dev,
    unsigned char loopback,
	unsigned char SCBR);

/*******************************************************************************************
 FUNCTION:    uspi_start
 DESCRIPTION:   start reading the SPI to the file
 PARAMS:        dev - handle to uspi device
                freq - frequency in Hz
                hsize - header size in bytes
                adcnum - number of ADC devices in the chain
                file - handle to file (must be opened for binary writing "wb"
                bufsize - size of buffer for usbb transfers
 RETURNS:     >0 on success
********************************************************************************************/
int uspi_start(uspi_handle* dev,
                unsigned spi,
                unsigned drdy,
                unsigned adcnum,
                unsigned hsize);



/*******************************************************************************************
 FUNCTION:    uspi_stop
 DESCRIPTION: stop reading SPI
 PARAMS:      dev - handle to uspi device
 RETURNS:     >0 on success
********************************************************************************************/
int uspi_stop(uspi_handle* dev);

/*******************************************************************************************
 FUNCTION:    uspi_getstat
 DESCRIPTION: stop reading SPI
 PARAMS:        dev - handle to uspi device
                stat - pointer to CHAN_STAT structure where stats will be written
 RETURNS:     >0 on success
********************************************************************************************/
int uspi_getstat(uspi_handle* dev, struct uspi_stat* stat);

int uspi_read(uspi_handle* uspi, struct uspi_sample *samples, unsigned count);




int uspi_i2c_write(
	uspi_handle* uspi,
	unsigned char dadr,
	unsigned char iadr,
	unsigned char data
	);

int uspi_i2c_read(
	uspi_handle* uspi,
	unsigned char dadr,
	unsigned char iadr,
	unsigned char *data
	);

int uspi_i2c_setspeed(
	uspi_handle* uspi,
	unsigned int speed
	);



#endif
