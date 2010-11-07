#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "uspi.h"

#define BUF_SIZE 10*1024


#define TIME 10

void usage()
{
    printf("Usage: uspi_test [options]\n");
    printf(" -header [headersize in bytes]\n");
    printf(" -spi [spi channel 0..3 or 4-loopback]\n");
    printf(" -nadc [number of spi devices in chain]\n");
    printf(" -scbr [SPI CLK divisor (base 48MHz)]\n");
    printf(" -edge [falling/rising]\n");
    printf(" -time [time of execution in sec]\n");
    printf(" -help  - display this help\n");
}

int main(int argc,char** argv)
{
    int ret,i,loop;
    //usb_dev_handle *dev = NULL; /* the device handle */
    clock_t t1,t2;
    FILE* f;
    CHAN_STAT stat;
    unsigned spi=0;
    unsigned drdy=2;
    unsigned hsize=4;
    unsigned dsize=1;
    unsigned time=20;
    unsigned count=0;
    unsigned loops=1;
    unsigned adcnum=1;
    unsigned scbr=1;
    unsigned arg=1;
    char bSpckInactiveHigh=0;
    char bSpckCaptureRising=1;

    while(arg<argc)
    {
        if(!strcmp(argv[arg],"-help"))
        {
            usage();
            return 0;
        }else
        if(!strcmp(argv[arg],"-header"))
        {
            arg++;
            hsize=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-spi"))
        {
            arg++;
            spi=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-drdy"))
        {
            arg++;
            drdy=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-nadc"))
        {
            arg++;
            adcnum=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-time"))
        {
            arg++;
            time=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-scbr"))
        {
            arg++;
            scbr=atoi(argv[arg]);
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-loops"))
        {
            arg++;
            loops=atoi(argv[arg]);
            arg++;
            continue;
        }
        else
        if(!strcmp(argv[arg],"-edge"))
        {
            arg++;
            if(!strcmp(argv[arg],"rising"))
                bSpckCaptureRising=1;
            else
                bSpckCaptureRising=0;
            arg++;
            continue;
        }else
        if(!strcmp(argv[arg],"-count"))
        {
            arg++;
            count=atoi(argv[arg]);
            arg++;
            continue;
        }else
        {
            usage();
            return 1;
        }
    }


    printf("Start header=%u bytes, spi channel=%u, nadc=%u, scbr =%u clocks, %s edge, time=%u sec, %u loops\n",
        hsize,spi,adcnum, scbr,(bSpckCaptureRising?"rising":"falling"), time,loops);

    uspi_handle* dev = uspi_open();
    if(!dev)
    {
        printf("Failed to open device\n");
        return 0;
    }

    ret=uspi_getmips(dev);
    if(ret)
        printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);

    uspi_setspi(dev,0,0,bSpckCaptureRising,scbr,1,0);
    for(loop=0;loop<loops;loop++)
    {
        char filename[10];
        sprintf(filename,"uspi%u.dat",loop);
        f=fopen(filename,"wb");
        if(!f)
        {
            printf("Failed to open %s\n",filename);
            break;
        }
        printf("Loop%u\n",loop);
        printf("  SENT         USB      TMR      SPI  MIPS\n");
        //ret=uspi_settimer(dev,freq);

        ret=uspi_start(dev,spi,drdy,adcnum,hsize,count,f,1024*64);
        if(!count){
            t1=clock();
            for(i=0;i<time*10;i++)
            {
                _sleep(95);
                ret=uspi_getstat(dev,&stat);
                ret=uspi_getmips(dev);
                if(ret)
                 printf("%8u %8u %8u %8u   %2u.%u%%\n",stat.PktSent,stat.UsbOvflw,stat.TmrMissCnt,stat.SpiOverRun,ret/10,ret%10);
            }
        }
        ret=uspi_stop(dev);
        t2=clock();
        fclose(f);
        ret=uspi_getstat(dev,&stat);
        if(ret)
            printf("%8u %8u %8u %8u\n",stat.PktSent,stat.UsbOvflw,stat.TmrMissCnt,stat.SpiOverRun);

        printf("Execution time %u\n",t2-t1);
    }
    _sleep(500);
    ret=uspi_getmips(dev);
    if(ret)
        printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);
    uspi_close(dev);
    return 0;

}
