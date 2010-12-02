#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
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
    struct uspi_stat stat;
    unsigned spi=0;
    unsigned drdy=2;
    unsigned hsize=4;
    unsigned time=20;
    unsigned count=0;
    unsigned loops=1;
    unsigned adcnum=1;
    unsigned scbr=1;
    unsigned arg=1;
    struct uspi_sample samples[1000];

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


    printf("Start header=%u bytes, spi channel=%u, nadc=%u, scbr =%u clocks, time=%u sec, %u loops\n",
        hsize,spi,adcnum, scbr, time,loops);

    uspi_handle* dev = uspi_open();
    if(!dev)
    {
        printf("Failed to open device\n");
        return 0;
    }

    ret=uspi_getmips(dev);
    if(ret)
        printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);

    uspi_setspi(dev,0,scbr);
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
        printf("  SENT         USB      SPI  MIPS\n");
        ret=uspi_start(dev,spi,drdy,adcnum,hsize);
        t1=clock();
        for(i=0;i<time*10;i++)
        {
            _sleep(30);
            ret=uspi_read(dev, samples, sizeof(samples)/sizeof(struct uspi_sample));
            if(ret)
                fwrite(samples,sizeof(struct uspi_sample),ret,f);
            //if(ret!=sizeof(samples)/sizeof(struct uspi_sample))
            //    printf();
            ret=uspi_getstat(dev,&stat);
            ret=uspi_getmips(dev);
            if(ret)
             printf("%8u %8u %8u   %2u.%u%%\n",stat.PktSent,stat.UsbOvflw,stat.SpiOverRun,ret/10,ret%10);
        }
        ret=uspi_stop(dev);
        t2=clock();
        fclose(f);
        ret=uspi_getstat(dev,&stat);
        if(ret)
            printf("%8u %8u %8u\n",stat.PktSent,stat.UsbOvflw,stat.SpiOverRun);

        printf("Execution time %u\n",t2-t1);
    }
    _sleep(500);
    ret=uspi_getmips(dev);
    if(ret)
        printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);
    uspi_close(dev);
    return 0;

}
