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
/*
 * OS independent serial interface
 *
 * Heavily based on Pirate-Loader:
 * http://the-bus-pirate.googlecode.com/svn/trunk/bootloader-v4/pirate-loader/source/pirate-loader.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>

#include "serial.h"
int serial_setup(int fd, speed_t speed)
{
	struct termios t_opt;

	/* set the serial port parameters */
	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &t_opt);
	/*
	t_opt.c_cflag |= (CLOCAL | CREAD);
	t_opt.c_cflag &= ~PARENB;
	t_opt.c_cflag &= ~CSTOPB;
	t_opt.c_cflag &= ~CSIZE;
	t_opt.c_cflag |= CS8;
	t_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	t_opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	t_opt.c_oflag &= ~OPOST;*/
	t_opt.c_cflag =  CS8 | CLOCAL | CREAD;
	t_opt.c_iflag = IGNPAR;
	t_opt.c_oflag = 0;
	t_opt.c_lflag = 0;
	cfsetispeed(&t_opt, speed);
	cfsetospeed(&t_opt, speed);
	t_opt.c_cc[VMIN] = 0;
	//t_opt.c_cc[VTIME] = 10;
	t_opt.c_cc[VTIME] = 1;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &t_opt);
	return 0;
}

const char *byte_to_binary ( int x )
{
	static char b[9];
	b[0] = '\0';

	int z;
	for (z = 256; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

int serial_write(int fd, char *buf, int size)
{
	int ret = 0;
	ret = write(fd, buf, size);

	//fprintf(stderr, "size = %d ret = %d\n", size, ret);
	//buspirate_print_buffer(buf, size);
	/*printf("Serial: Write %i bytes:  ",ret);
	int i;
	for (i = 0; i < ret; i++){
		printf("%d | 0x%02X | %s\n", buf[i], buf[i], byte_to_binary(buf[i]));
	}
	*/

   	if (ret != size)
	    	fprintf(stderr, "Error sending data\n");
	return ret;
}

int serial_read(int fd, char *buf, int size)
{
	int len = 0;
	int ret = 0;
	int timeout = 0;

	while (len < size) {
		ret = read(fd, buf+len, size-len);
		if (ret == -1){
			return -1;
		}

		if (ret == 0) {
			timeout++;
			if (timeout > 1)
				break;

			continue;
		}
		/*printf("Serial: Read %i bytes:  ",ret);
		int i;
		for (i = 0; i < ret; i++){
			printf("%d | 0x%02X | %s\n", buf[i], buf[i], byte_to_binary(buf[i]));
		}
		*/
		len += ret;
	}
	return len;
}



int serial_open(char *port)
{
	int fd;
	fd = open(port, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		fprintf(stderr, "Could not open serial port.");
		return -1;
	}
	return fd;
}

int serial_close(int fd)
{
	close(fd);
	return 0;
}

/*
int readWithTimeout(int fd, uint8_t *out, int length, int timeout)
{
	fd_set fds;
	struct timeval tv = {timeout, 0};
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
#ifdef WIN32
	COMMTIMEOUTS timeouts;
	DCB dcb = {0};
	HANDLE hCom = (HANDLE)fd;

	dcb.DCBlength = sizeof(dcb);

	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if( !SetCommState(hCom, &dcb) ){
		return -1;
	}


	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 100;

	if (!SetCommTimeouts(hCom, &timeouts)) {
		return -1;
	}

	return (int)hCom;
#else
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

#ifdef MACOSX

	if( tcsetattr(fd, TCSANOW, &g_new_tio) < 0 ) {
		return -1;
	}

	return ioctl( fd, IOSSIOSPEED, &baud );
#else
	cfsetispeed (&g_new_tio, baudrate);
	cfsetospeed (&g_new_tio, baudrate);

	tcflush(fd, TCIOFLUSH);

	return tcsetattr(fd, TCSANOW, &g_new_tio);
#endif //#ifdef MACOSX

#endif
}

int openPort(const char* dev, unsigned long flags)
{
	return open(dev, O_RDWR | O_NOCTTY | O_NDELAY | flags);
}
*/
