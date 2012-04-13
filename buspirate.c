#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "serial.h"
#include "buspirate.h"




int BP_EnableBinary(int fd)   // should return BBIO if ok, ERR if not
{
	int ret;
	char tmp[100] = { [0 ... 20] = 0x00 };
	int done = 0;
	//int cmd_sent = 0;
	int tries=0;

	printf(" Entering binary mode...\n");

	if (fd==-1)   //added because the fd has already returned null
	{
		printf("Port does not exist!");
		ret=ERR;

	} 
	else
	{
		while (!done) {
			tmp[0]=0x00;
			serial_write(fd, tmp, 1);
			tries++;
			ret = serial_read(fd, tmp, 5);
			if (ret != 5 && tries>20) {
				fprintf(stderr, " Buspirate did not respond correctly :( %i \n", ret );
				ret=ERR;
			}
			else if (strncmp(tmp, "BBIO1", 5) == 0) {
				done=1;
				ret=BBIO;
			}

			if (tries>25){
				printf(" Buspirate:Too many tries in serial read! -exiting \n - chip not detected, or not readable/writable\n");
				ret=ERR;
				break;
			}
		}
	}
	return ret;
}

