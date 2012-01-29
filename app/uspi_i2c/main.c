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
	printf("Usage: uspi_i2c -a DADR -i IADR [-w DATA] [-s I2C_SPEED_HZ]\n");
	printf(" DADR = 7bit I2C Slave Address (00..7F)\n");
	printf(" IADR = I2C Slave Internal Address (00..FF)\n");
	printf(" DATA = Data to write (00..FF)\n");
	printf(" I2C_SPEED_HZ = I2C bus speed (10000..400000)\n");
}

int main(int argc,char** argv)
{
	int ret;
	unsigned arg=1;
	int data=-1;
	int dadr=-1;
	int iadr=-1;
	int speed=-1;
	uspi_handle* dev;


	while(arg<argc)
	{
		if(!strcmp(argv[arg],"-help") || !strcmp(argv[arg],"-h"))
		{
			usage();
			return 0;
		}else
		if(!strcmp(argv[arg],"-a"))
		{
			arg++;
		sscanf(argv[arg],"%x",&dadr);
			arg++;
			continue;
		}else
		if(!strcmp(argv[arg],"-i"))
		{
			arg++;
			sscanf(argv[arg],"%x",&iadr);
			arg++;
			continue;
		}else
		if(!strcmp(argv[arg],"-w"))
		{
			arg++;
			sscanf(argv[arg],"%x",&data);
			arg++;
			continue;
		}else
		if(!strcmp(argv[arg],"-s"))
		{
			arg++;
			speed=atoi(argv[arg]);
			arg++;
			continue;
		}else
		{
			usage();
			return 1;
		}

	}
	if( iadr<0 || dadr<0 || speed<10000 || speed>400000 ) {
			usage();
			return 1;
	}

	printf("%s I2C dadr=0x%x iadr=0x%x Speed=%uHz\n",(data<0?"Read":"Write"),dadr,iadr,speed);


	dev = uspi_open();
	if(!dev)
	{
		printf("Failed to open device\n");
		return 0;
	}

	ret=uspi_getmips(dev);
	if(ret)
		printf("Free MIPS in idle: %2u.%u%%\n",ret/10,ret%10);

	ret=uspi_i2c_setspeed(dev,speed);
	
	if(data>0)
	ret=uspi_i2c_write(dev,dadr,iadr,data);
	else
	ret=uspi_i2c_read(dev,dadr,iadr,&data);

	printf("Error=%d data=0x%02x\n",ret,(unsigned char)data);

	uspi_close(dev);
	return 0;

}
