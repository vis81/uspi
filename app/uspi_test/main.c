#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "uspi.h"

#define BUF_SIZE 10000


#define TIME 10

void usage()
{
    printf("Usage: uspi_test [options]\n");
    //printf(" -header [headersize in bytes]\n");
    printf(" -spi [spi channel 0..3]\n");
    printf(" -drdy [used nDRDY pin 0..3]\n");
    printf(" -nadc [number of spi devices in chain]\n");
    printf(" -scbr [SPI CLK divisor (base 48MHz)]\n");
    printf(" -spilb - enable internal SPI loopback\n");
    printf(" -time [time of execution in ms]\n");
    printf(" -count [number of samples to read, ovverrides -time]\n");
    printf(" -loops [number of loops to execute]\n");
    printf(" -help  - display this help\n");
}

int main(int argc,char** argv)
{
    int ret,loop;
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
    unsigned spi_lb=0;
    unsigned arg=1;
    struct uspi_sample samples[BUF_SIZE];

    while(arg<argc)
    {
        if(!strcmp(argv[arg],"-help"))
        {
            usage();
            return 0;
        }else
        /*
        if(!strcmp(argv[arg],"-header"))
        {
            arg++;
            hsize=atoi(argv[arg]);
            arg++;
            continue;
        }else
        */
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
        if(!strcmp(argv[arg],"-spilb"))
        {
            arg++;
            spi_lb=1;
            continue;
        }else
        {
            usage();
            return 1;
        }
    }

    if(!count)
        count = time * 32;
    else
        time = count / 32;

    printf("Starting USPI test\n");
    printf("SPI channel :%u\n", spi);
    printf("Active DRDY :%u\n", drdy);
    printf("ADC count   :%u\n", adcnum);
    printf("SCBR        :%u clocks\n", scbr);
    printf("SPI loopback:%s\n", spi_lb ? "true" : "false");
    printf("Header size :%u bytes\n", hsize);
    printf("Loops       :%u\n", loops);
    printf("Time        :%u ms\n", time);
    printf("Count       :%u samples\n", count);
    printf("\n");


    uspi_handle* dev = uspi_open();
    if(!dev)
    {
        printf("Failed to open device\n");
        return 0;
    }

    ret=uspi_getmips(dev);
    if(ret)
        printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);

    uspi_setspi(dev,spi_lb,scbr);
    for(loop=0;loop<loops;loop++)
    {
        char filename[10];
        unsigned total=0;
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
        while(total<count)
        {
            unsigned left = count - total;
            ret=uspi_read(dev, samples, BUF_SIZE > left ? left : BUF_SIZE);
            if(!ret)
                break;
            fwrite(samples,sizeof(struct uspi_sample),ret,f);
            total+=ret;
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
