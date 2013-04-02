#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

//char *portname = "/dev/ttyS0";
char *portname = "/dev/ttyAMA0";

static int serial_fd;

// opens and configures the serial port, returns its file descriptor, stores its file descriptor
int32_t serial_open(void) {
	// open the port
	serial_fd = open (portname, O_RDWR| O_NOCTTY | O_SYNC | O_NONBLOCK);
	if (serial_fd < 0)
	{
		perror("While opening read serial port ");
		return -1;
	}

	struct termios options;
	tcgetattr(serial_fd, &options);
	cfsetispeed(&options,B57600);
	cfsetospeed(&options,B57600);
	options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	tcflush(serial_fd, TCIFLUSH);
	tcsetattr(serial_fd,TCSANOW, &options);
    
	return serial_fd;
}

int32_t serial_write(uint8_t *buf, uint32_t len) {
  int n = 0;
  struct timeval tv;
  fd_set wfds;
  int ret;
  printf("write called len %d\n",len);
  // build the descriptor list for the select
  FD_ZERO(&wfds);
  FD_SET(serial_fd,&wfds);
  tv.tv_sec = 0;
  tv.tv_usec = 50000;
  while (len > n) {
    printf("in while");
    ret = select(serial_fd+1, NULL, &wfds, NULL, NULL);
    n += write(serial_fd,&buf[n],len);
    printf ("wrote serial %d", n);
  }
  return n;
}
