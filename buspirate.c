#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "serial.h"
#include "buspirate.h"

#define OOCD_CMD_UNKNOWN	0x00
#define OOCD_CMD_PORT_MODE	0x01
#define OOCD_CMD_FEATURE	0x02
#define OOCD_CMD_READ_ADCS	0x03
#define OOCD_CMD_TAP_SHIFT	0x05
#define OOCD_CMD_ENTER_OOCD	0x06 // this is the same as in binIO
#define OOCD_CMD_UART_SPEED	0x07
#define OOCD_CMD_JTAG_SPEED	0x08

#define MAX_BUF_LEN 1024
static unsigned char buf[2*MAX_BUF_LEN];
unsigned char tdobuf[MAX_BUF_LEN];

int BP_EnableBinary(int fd)   // should return BBIO if ok, ERR if not
{
	int ret;
	int done = 0;
	int tries=0;

	printf(" Entering binary mode...\n");

	if (fd==-1) { //added because the fd has already returned null
		printf("Port does not exist!\n");
		ret=ERR;

	} else {
		while (!done) {
			buf[0] = 0x00;
			serial_write(fd, buf, 1);
			tries++;
			ret = readWithTimeout(fd, buf, 5, 10000);
			if (ret != 5 && tries>20) {
				fprintf(stderr, " Buspirate did not respond correctly :( %i \n", ret );
				ret = ERR;
			} else if (strncmp(buf, "BBIO1", 5) == 0) {
				done = 1;
				ret = BBIO;
			}

			if (tries>25) {
				printf("Buspirate:Too many tries in serial read! -exiting \n - chip not detected, or not readable/writable\n");
				ret = ERR;
				break;
			}
		}
	}
	return ret;
}

void BP_DisableBinary(int fd)
{
	int ret;
	int done = 0;
	int tries=0;

	printf(" Exiting binary mode...\n");

	if (fd==-1) { //added because the fd has already returned null
		printf("Port does not exist!\n");

	} else {
		while (!done) {
			buf[0] = 0x00;
			serial_write(fd, buf, 1);
			tries++;
			ret = readWithTimeout(fd, buf, 5, 100000);
			if (ret != 5 && tries>20) {
				fprintf(stderr, " Buspirate did not respond correctly :( %i \n", ret );
			} else if (strncmp(buf, "BBIO1", 5) == 0) {
				done = 1;
			}

			if (tries>25) {
				printf("Buspirate:Too many tries in serial read! -exiting \n - chip not detected, or not readable/writable\n");
				break;
			}
		}
		buf[0] = 0x0F; //exit binary mode
		serial_write(fd, buf, 1);
	}
}

int BP_EnableOCD(int fd)
{
	int ret;
	
	printf("Entering OpenOCD mode...");
	buf[0] = 0x06;
	serial_write(fd, buf, 1);
	ret = readWithTimeout(fd, buf, 4, 100000);
	if (ret == 4 && (strncmp(buf, "OCD1", 4) == 0)) {
		printf(" OK.\n");
		return OOCD;
	}
	return ERR;
}

void BP_OCDMode(int fd, int mode)
{
	buf[0] = OOCD_CMD_PORT_MODE;
	buf[1] = mode;
	serial_write(fd, buf, 2);
}

void BP_OCDFeature(int fd, int feature, int value)
{
	buf[0] = OOCD_CMD_FEATURE;
	buf[1] = feature;
	buf[2] = value;
	serial_write(fd, buf, 3);	
}

void print_buffer(unsigned char * buf, int count)
{
	unsigned int i;
	for (i=0; i<count; i++)
		printf("%02X", (unsigned char)buf[i]);
	printf("\n");
}


int BP_OCDTapShift(int fd, unsigned char *tdodata, const unsigned char *tdidata, const unsigned char *tmsdata, int bitcount)
{
	unsigned char tmp[3];
	int i,j,bytecount,howmuch,retry;
	
	/* 
	 * TODO: Add support for splitting for bitcnt > 0x2000
	 *
	 */
	buf[0] = OOCD_CMD_TAP_SHIFT;
	buf[1] = (char)(bitcount >> 8);
	buf[2] = (char)bitcount;
	serial_write(fd, buf, 3);
	
	i = readWithTimeout(fd, tmp, 3, 100000);
	if (i != 3 || (strncmp(buf, tmp, 3) != 0)) {
		printf("Didn't get correct response for TAP_SHIFT command.\n");
		exit(-1);
	}
	bytecount = (bitcount+7)/8;
	i = 0; j = 0;
	while (i < bytecount) {
		buf[j++] = tdidata[i];
		buf[j++] = tmsdata[i++];
	}
	/* Send and read by 64-bytes chunks*/
	i = 0;
	while (i < 2*bytecount) {
		howmuch = ((2*bytecount - i) > 64) ? 64 : 2*bytecount - i;
		//printf("Out: ");
		//print_buffer(buf+i, howmuch);
		j = serial_write(fd, buf+i, howmuch);
		if (j > 0) {
			i += j;
			//printf("sent: %d\n",i);
		}
	}

	i = 0;
	retry = 0;
	while (i < bytecount) {
		if (retry > bytecount) {
			printf("Didn't get all the data.\n");
			break;
		}
		howmuch = ((bytecount - i) > 64) ? 64 : bytecount - i;
		j = readWithTimeout(fd, tdodata+i, howmuch, 100000);
		if (j > 0) {
			//printf("In: ");
			//print_buffer(tdodata+i,j);
			i += j;
			retry = 0;
			//printf("read %d\n",i);
		} else {
			//printf("retry: %d\n",retry);
			retry++;
		}
	}
	
	return i;
}

