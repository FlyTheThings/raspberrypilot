#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

//project specific
#include <uavlink.h>
#include <uavlink_server_serial.h>
#include <uavlink_server_utils.h>

//char *portname = "/dev/ttyS0";
char *portname = "/dev/ttyAMA0"; //this is hardcoded since its just on the raspberry pi (for now)

static int serial_fd;

#define UAVLINK_READ_BUFFER_LEN 255
static uint8_t uavlink_read_buffer[UAVLINK_READ_BUFFER_LEN];

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
  fd_set wfds;
  // build the descriptor list for the select
  FD_ZERO(&wfds);
  FD_SET(serial_fd,&wfds);
  while (len > n) {
    select(serial_fd+1, NULL, &wfds, NULL, NULL);
    n += write(serial_fd,&buf[n],len);
  }
  return n;
}

void handle_serial_rx(int fd_uavlink_serial, UAVLinkConnection uav_link_conn) {
     int n = read(fd_uavlink_serial, uavlink_read_buffer, UAVLINK_READ_BUFFER_LEN);
     if (n < 0) {
       perror ("Error rx\n");
       exit(-1);
     }
     int i = 0;
     while (n>0) {
	   UAVLinkProcessInputStream(uav_link_conn, uavlink_read_buffer[i]);
	   i++;
	   n--;
     }
}

bool wait_uavlink_response(UAVLinkConnection uav_link_conn, uint8_t *buf, uint16_t *len) {
  // returns true if an response was received
  struct timeval tv;
  uint64_t end_time;
  int64_t wait_time;
  fd_set rfds;
  int selret;
  end_time = get_time_stamp() + 50000;  //timeout in 50ms
  while(1) {
    wait_time = end_time - get_time_stamp();
    printf ("wait time: %llu\n",wait_time);
    // if we timeout out return 0
    if (wait_time < 0) {
      printf("wait time too short");
      return 0;
    }
    tv.tv_usec = wait_time;
    tv.tv_sec = 0;
    FD_ZERO(&rfds);
    FD_SET(serial_fd,&rfds);
    selret = select(serial_fd+1,&rfds,NULL,NULL,&tv);
    // if it timed out return 0
    if (selret == 0) {
      printf("sel timeout\n");
      return 0;
    }
    // now process the serial data one byte at a time 
    read(serial_fd, uavlink_read_buffer, 1);
    UAVLinkProcessInputStream(uav_link_conn,uavlink_read_buffer[0]);
    // check if there was a response
    if (UAVLinkGetResponsePacket(uav_link_conn,buf,len)) return 1;
  }
} 

