/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project and http://dangerousprototypes.com
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "serial.h"

int serial_write(int fd, const unsigned char *buf, int size)
{
	int ret = 0;

	ret = write(fd, buf, size);
   	if (ret != size)
	    	fprintf(stderr, "Error sending data\n");
	return ret;
}

int readWithTimeout(int fd, unsigned char *out, int length, int timeout)
{
	fd_set fds;
	// timeout is in microseconds
	struct timeval tv = {0, timeout};
	int res = -1;
	int got = 0;

	do {

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		res = select(fd + 1, &fds, NULL, NULL, &tv);

		if( res > 0 ) {
			res = read(fd, out, length);
			if( res > 0 ) {
				length -= res;
				got    += res;
				out    += res;
			} else {
				break;
			}
		} else {
			return res;
		}
	} while( length > 0);

	return got;
}

int configurePort(int fd, unsigned long baudrate)
{
    speed_t baud = B921600;
	struct termios g_new_tio;

	switch (baudrate) {
		case 921600:
			baud = B921600;
			break;
		case 115200:
			baud = B115200;
			break;
		case 1000000:
			baud = B1000000;
			break;
		case 1500000:
			baud = B1500000;
		default:
			printf("unknown speed setting \n");
			return -1;
			break;
	}

	memset(&g_new_tio, 0x00 , sizeof(g_new_tio));
	cfmakeraw(&g_new_tio);

	g_new_tio.c_cflag |=  (CS8 | CLOCAL | CREAD);
	g_new_tio.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	g_new_tio.c_oflag = 0;
	g_new_tio.c_lflag = 0;

	g_new_tio.c_cc[VTIME] = 0;
	g_new_tio.c_cc[VMIN] = 1;

	cfsetispeed (&g_new_tio, baud);
	cfsetospeed (&g_new_tio, baud);

	tcflush(fd, TCIOFLUSH);

	return tcsetattr(fd, TCSANOW, &g_new_tio);
}

int openPort(const char* dev, unsigned long flags)
{
	return open(dev, O_RDWR | O_NOCTTY | O_NDELAY | flags);
}

int closePort(int fd)
{
	close(fd);
	return 0;
}
