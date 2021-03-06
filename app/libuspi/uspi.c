#include <usb.h>
#include <process.h>
#include <stdio.h>
#include <fcntl.h>
#include "uspi.h"
#include "../../FW/commands.h"


#define MY_VID 0xe463
#define MY_PID 0x0007

#define PIPE_SIZE 2*1024*1024
#define BUF_SIZE 13*64*100
#define USB_TIMEOUT 1000

enum PIPES { READ, WRITE }; /* Constants 0 and 1 for READ and WRITE */

struct uspi_handle{
    usb_dev_handle* dev;
    int fdpipe[2];
    HANDLE hThread;
    int rx_bytes;
    int tx_bytes;
};

unsigned bread,bwrite;
static usb_dev_handle *open_dev(void)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for(bus = usb_get_busses(); bus; bus = bus->next)
    {
      for(dev = bus->devices; dev; dev = dev->next)
        {
          if(dev->descriptor.idVendor == MY_VID
             && dev->descriptor.idProduct == MY_PID)
            {
              return usb_open(dev);
            }
        }
    }
  return NULL;
}

uspi_handle* uspi_open()
{
    usb_dev_handle* dev;
    uspi_handle* uspih;
    char tmp[30];
    unsigned ret;

    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */

    if(!(dev = open_dev()))
    {
        printf("error: device not found!\n");
        return NULL;
    }

    if(usb_set_configuration(dev, 1) < 0)
    {
      printf("error: setting config 1 failed\n");
      usb_close(dev);
      return NULL;
    }

     if(usb_claim_interface(dev, 0) < 0)
    {
      printf("error: claiming interface 0 failed\n");
      usb_close(dev);
      return NULL;
    }

    ret=usb_get_string_simple(dev, 1, tmp, USB_TIMEOUT);
    if(ret){
        tmp[ret]=0;
        printf("Manufacturer:%s\n",tmp);
    }

    ret=usb_get_string_simple(dev, 2, tmp, USB_TIMEOUT);
    if(ret){
        tmp[ret]=0;
        printf("Product:%s\n",tmp);
    }
    ret=usb_get_string_simple(dev, 3, tmp, USB_TIMEOUT);
    if(ret){
        tmp[ret]=0;
        printf("Version:%s\n",tmp);
    }
    uspih = (uspi_handle*)malloc(sizeof(uspi_handle));
    uspih->dev=dev;
    return uspih;
}

void uspi_close(uspi_handle* uspi)
{
    if(usb_release_interface(uspi->dev, 0) < 0)
        printf("error: release interface 0 failed\n");
    usb_close(uspi->dev);
}

int uspi_fw_version(uspi_handle* uspi,char* buf, unsigned size)
{
    return usb_get_string_simple(uspi->dev, 3, buf, size);
}

int uspi_getmips(uspi_handle* uspi)
{
    unsigned mips=-1;
    usb_control_msg(uspi->dev, USB_TYPE_VENDOR|0x80, CMD_GETMIPS, 0, 0, (char*)&mips, sizeof(mips), USB_TIMEOUT);
    return mips;
}

static void uspi_thread(void* param)
{
    uspi_handle* uspi=(uspi_handle*) param;
    int ret;
    char* pbuf=malloc(BUF_SIZE);
    if(!pbuf)
        goto exit;

    if(!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
   {
      ret = GetLastError();
      if( REALTIME_PRIORITY_CLASS == ret)
         printf("Already in background mode\n");
      else printf("Failed to enter background mode (%d)\n", ret);
   }

    if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
   {
      ret = GetLastError();
      if( THREAD_PRIORITY_TIME_CRITICAL == ret)
         printf("Already in background mode\n");
      else
        printf("Failed to enter background mode (%d)\n", ret);
   }

    while (1) {
        ret=usb_bulk_read(uspi->dev, 0x81,pbuf,BUF_SIZE, USB_TIMEOUT);
        if(ret>0){
            int ret2=write(uspi->fdpipe[WRITE], pbuf, ret);
            if(ret!=BUF_SIZE || ret2!=BUF_SIZE){
                break;
            }
            bwrite+=BUF_SIZE;
        }else
            break;
    };
    free(pbuf);
exit:
    _endthread();
    return;
}

int uspi_read(uspi_handle* uspi, struct uspi_sample *samples, unsigned count)
{
    int ret;
    unsigned total=0;
    count*=sizeof(struct uspi_sample);
    while(count){
        ret = read( uspi->fdpipe[READ], ((char *)samples)+total, count);
        if(ret<0)
            break;
        total+=ret;
        count-=ret;
    }
    return total/sizeof(struct uspi_sample);
}

double convert(unsigned char* raw){
    double double_val;
    unsigned char *padcval;
    int int_val;

    padcval=(unsigned char*)raw;
    int_val=((padcval[0]<<16)&0xFF0000) |
            ((padcval[1]<<8)&0xFF00) |
            (padcval[2]&0xFF);
    if(int_val&0x800000)//extend sign
    int_val|=0xFF000000;

    if(int_val)
        double_val=int_val*5.0/16777216.0;
    else
        double_val=0.0;
    return double_val;
}

int uspi_read_wave(uspi_handle* uspi, double* data0, double* data1, double* data2, unsigned count)
{
    int ret;
    unsigned total=0;
    struct uspi_sample sample;
    unsigned ts, first_ts,miss=0;

    while(total<count){
        double d0,d1,d2;

        if(!miss)
            ret = read( uspi->fdpipe[READ], (char *)&sample, sizeof(struct uspi_sample));
        bread+=sizeof(struct uspi_sample);
        if(ret<0)
            break;
        if ( sample.time == ts+1 || total==0)
        {
            ts=sample.time;
            d0=convert(sample.data[0]);
            d1=convert(sample.data[1]);
            d2=convert(sample.data[2]);
            miss=0;
        } else {
            //miss
            miss=1;
            d0 = d1 = d2 = -10.0;
            ts += 1;
        }
        if(data0)
            data0[total]=d0;
        if(data1)
            data1[total]=d1;
        if(data2)
            data2[total]=d2;

        if(total==0)
            first_ts = ts;
        total++;
    }
    return first_ts;
}

int uspi_start(uspi_handle* uspi,
                unsigned spi,
                unsigned drdy,
                unsigned adcnum,
                unsigned hsize)
{
    unsigned ret;
    struct cmd_start_in params;

    bwrite=bread=0;
    /* Open a set of pipes */
    if( _pipe( uspi->fdpipe, PIPE_SIZE, O_BINARY ) == -1 )
          return errno;
    params.spi=spi;
    params.drdy=drdy;
    params.adcnum=adcnum;
    params.hsize=hsize;
    usb_resetep(uspi->dev, 0x81);
    ret=usb_control_msg(uspi->dev, USB_TYPE_VENDOR, CMD_START, 0, 0, (void*)&params,sizeof(params), USB_TIMEOUT);
    if(ret!= sizeof(params))
      return -1;
    uspi->hThread = (HANDLE)_beginthread(uspi_thread, 0,uspi);
    return 0;
}


int uspi_stop(uspi_handle* uspi)
{
    unsigned ret;
    ret=usb_control_msg(uspi->dev, USB_TYPE_VENDOR, CMD_STOP, 0, 0, NULL, 0, USB_TIMEOUT);
    close(uspi->fdpipe[READ]);
    close(uspi->fdpipe[WRITE]);
    WaitForSingleObject( uspi->hThread, USB_TIMEOUT );
    uspi->fdpipe[READ]=uspi->fdpipe[WRITE]=0;
    uspi->hThread=NULL;
    return ret;
}

int uspi_getstat(uspi_handle* uspi,struct uspi_stat *stat)
{
    if(!stat)
        return -1;
    stat->BufLevel=bwrite-bread;
    return usb_control_msg(uspi->dev, USB_TYPE_VENDOR|0x80, CMD_GETSTAT, 0, 0, (char*)stat, 12, USB_TIMEOUT);
}

int uspi_setspi(uspi_handle* uspi,
    unsigned char loopback,
	unsigned char SCBR)
{
    struct cmd_setspi_in params;
    params.loopback=loopback;
    params.scbr=SCBR;
    return usb_control_msg(uspi->dev, USB_TYPE_VENDOR, CMD_SETSPI, 0, 0, (char*) &params, sizeof(params), USB_TIMEOUT);
}

int uspi_i2c_write(
	uspi_handle* uspi,
	unsigned char dadr,
	unsigned char iadr,
	unsigned char data
	)
{
    struct cmd_i2c_out out;
    int ret;
    ret = usb_control_msg(uspi->dev, USB_TYPE_VENDOR|0x80, CMD_I2C, data | 0x100, (iadr<<8) | dadr, (char*) &out, 2, USB_TIMEOUT);
    if (ret!=2)
		return ret;
    else
		return out.err;
}

int uspi_i2c_read(
	uspi_handle* uspi,
	unsigned char dadr,
	unsigned char iadr,
	unsigned char *data
	)
{
    struct cmd_i2c_out out;
    int ret;
    ret = usb_control_msg(uspi->dev, USB_TYPE_VENDOR|0x80, CMD_I2C, 0, (iadr<<8) | dadr, (char*) &out, 2, USB_TIMEOUT);
    if (ret!=2)
		return ret;
    *data=out.data;
    return out.err;
}

int uspi_i2c_setspeed(
	uspi_handle* uspi,
	unsigned int speed
	)
{
    int ret;
    ret = usb_control_msg(uspi->dev, USB_TYPE_VENDOR, CMD_I2C_SPEED, 0, 0, (char*) &speed, 4, USB_TIMEOUT);
    return ret;
}
