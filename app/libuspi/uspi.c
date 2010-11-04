#include "uspi.h"
#include "../../FW/commands.h"
#include <usb.h>
#include "process.h"
#include "stdio.h"
#include "windows.h"

#define MY_VID 0xe463
#define MY_PID 0x0007



typedef struct _uspi_thread_params{
    uspi_handle* dev;
    FILE* f;
    unsigned bufsize;
    unsigned totalsize;
}uspi_thread_params;

HANDLE hThread;
uspi_thread_params up;


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
    char tmp[30];
    unsigned ret;

    if(!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
   {
      ret = GetLastError();
      if( REALTIME_PRIORITY_CLASS == ret)
         printf("Already in background mode\n");
      else printf("Failed to enter background mode (%d)\n", ret);
   }

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

    ret=usb_get_string_simple(dev, 1, tmp, 100);
    if(ret){
        tmp[ret]=0;
        printf("Manufacturer:%s\n",tmp);
    }

    ret=usb_get_string_simple(dev, 2, tmp, 100);
    if(ret){
        tmp[ret]=0;
        printf("Product:%s\n",tmp);
    }
    ret=usb_get_string_simple(dev, 3, tmp, 100);
    if(ret){
        tmp[ret]=0;
        printf("Version:%s\n",tmp);
    }
    return (uspi_handle*)dev;
}

void uspi_close(uspi_handle* dev)
{
    usb_close((usb_dev_handle*)dev);
}

int uspi_getmips(uspi_handle* dev)
{
    unsigned ret;
    char tmp[4];
    ret=usb_control_msg((usb_dev_handle*)dev, USB_TYPE_VENDOR|0x80, CMD_GETMIPS, 0, 0, tmp, 4, 10000);
    if(ret!= 4)
        ret=-1;
    else
        ret=*(int*)tmp;
    return ret;
}

void uspi_thread(uspi_thread_params* up)
{
    int ret;
    unsigned total=0;
    char* pbuf=malloc(up->bufsize);
    if(!pbuf)
        goto exit;

    if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
   {
      ret = GetLastError();
      if( THREAD_PRIORITY_TIME_CRITICAL == ret)
         printf("Already in background mode\n");
      else
        printf("Failed to enter background mode (%d)\n", ret);
   }

    do{
        ret=usb_bulk_read((usb_dev_handle*)up->dev, 0x81,pbuf,up->bufsize, 10000);
        if(ret>0){
            total+=ret;
            if(up->totalsize)
                if(total>up->totalsize)
                {
                    fwrite(pbuf,ret-(total-up->totalsize),1,up->f);
                    break;
                }

            fwrite(pbuf,ret,1,up->f);
        }
    }while(ret==up->bufsize);
    //printf("exit %u\n",ret);
exit:
    _endthread();
    return;
}

int uspi_start(uspi_handle* dev,
                unsigned spi,
                unsigned drdy,
                unsigned adcnum,
                unsigned hsize,
                unsigned count,
                FILE* file,
                unsigned bufsize)
{
    unsigned ret;
    char tmp[5];

    tmp[0]=spi;
    tmp[1]=drdy;
    tmp[2]=adcnum;
    tmp[3]=hsize;
    //tmp[4]=dsize;

    //usb_resetep((usb_dev_handle*)dev, 0x81);//doesn't work

    ret=usb_control_msg((usb_dev_handle*)dev, USB_TYPE_VENDOR, CMD_START, 0, 0, tmp,5, 1000);
    if(ret!= 5)
      return -1;

    up.dev=dev;
    up.f=file;
    up.bufsize=bufsize;
    up.totalsize=count*(hsize+9);
    if(count)
        up.bufsize=64;

    hThread=_beginthread(uspi_thread, 0,&up);
    if(count)
        WaitForSingleObject( hThread, INFINITE );
    return 0;
}

int uspi_stop(uspi_handle* dev)
{
    unsigned ret;
    ret=usb_control_msg((usb_dev_handle*)dev, USB_TYPE_VENDOR, CMD_STOP, 0, 0, NULL, 0, 10000);
    if(ret!= 0)
      return -1;
    //usb_resetep(dev, usb);
    WaitForSingleObject( hThread, INFINITE );
    return 0;
}

int uspi_getstat(uspi_handle* dev,CHAN_STAT* stat)
{
    unsigned ret;
    ret=usb_control_msg((usb_dev_handle*)dev, USB_TYPE_VENDOR|0x80, CMD_GETSTAT, 0, 0, stat, sizeof(CHAN_STAT), 10000);
    return ret;
}

int uspi_setspi(uspi_handle* dev,
    spi_mode_t spimode,
	char bSpckInactiveHigh,
	char bSpckCaptureRising,
	char SCBR,
	char DLYBS,
	char DLYBCT)
{
    char tmp[4];
    unsigned ret;
    tmp[0]=(spimode&0x7)|
            (bSpckInactiveHigh?0x10:0)|
            (bSpckCaptureRising?0x20:0);
    tmp[1]=SCBR;
    tmp[2]=DLYBS;
    tmp[3]=DLYBCT;
    ret=usb_control_msg((usb_dev_handle*)dev, USB_TYPE_VENDOR, CMD_SETSPI, 0, 0, tmp, 4, 10000);
    return ret;
}
