#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/* get current number of MIPS*/
#define	CMD_GETMIPS		0x00

struct cmd_getmips_out{
	unsigned int	curmips;
};


/* start channel*/
#define	CMD_START		0x01

struct cmd_start_in{
	unsigned char	spi;
	unsigned char	drdy;
	unsigned char	adcnum;
	unsigned char	hsize;
};


/* stop channel*/
#define	CMD_STOP		0x02


/* get stats*/
#define	CMD_GETSTAT		0x03

struct cmd_getstat_out {
	unsigned int PktSent;
	unsigned int UsbOvflw;
	unsigned int SpiOverRun;
};


/* set spi config*/
#define	CMD_SETSPI		0x04

struct cmd_setspi_in {
	unsigned char loopback;
	unsigned char scbr;
};


#define	NUM_CMDS		0x05

#endif
