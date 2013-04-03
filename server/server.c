// standard include files
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

// project include files
#include <serial.h>
#include <uavlink.h>

#define UAVLINK_READ_BUFFER_LEN 1
#define UDP_MSG_BFR_LEN 512

static uint8_t uavlink_read_buffer[UAVLINK_READ_BUFFER_LEN];
static uint8_t mesg_buffer[UDP_MSG_BFR_LEN];


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

uint64_t get_time_stamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}


bool wait_uavlink_ack(UAVLinkConnection uav_link_conn, int serial_fd) {
  // returns true if an ack was received
  struct timeval tv;
  uint64_t end_time;
  int64_t wait_time;
  bool waiting = 1;
  fd_set rfds;
  int selret;
  end_time = get_time_stamp() + 50000;  //timeout in 50ms
  while(waiting) {
    wait_time = end_time - get_time_stamp();
    // if we timeout out return 0
    if (wait_time < 0) return 0;
    tv.tv_usec = wait_time;
    tv.tv_sec = 0;
    selret = select(2,&rfds,NULL,NULL,&tv);
    // if it timed out return 0
    if (selret == 0) return 0;
    // now process the serial data one byte at a time 
    read(serial_fd, uavlink_read_buffer, 1);
    UAVLinkProcessInputStream(uav_link_conn,uavlink_read_buffer[0]);
    // check if there was a response
    if (UAVLinkGetResponse(uav_link_conn,NULL,NULL)) return 1;
  }
} 





int main(int argc, char**argv)
{
   int sock_fd;
   int uavlink_serial_fd;
   int max_fd;
   struct timeval tv;
   fd_set rfds;
   int selret;
   UAVLinkConnection uav_link_conn;
   printf("opening socket\n");
   
   //open the UDP port
   sock_fd = open_socket_uavlink();
   printf("opening serial ports\n");
   // open the serial port
   uavlink_serial_fd = serial_open();

   printf("configure uavlink\n");
   // configure uavlink session on serial port
   uav_link_conn = UAVLinkInitialize( (UAVLinkOutputStream) serial_write);

   max_fd = 0;
   max_fd = uavlink_serial_fd > max_fd ? uavlink_serial_fd : max_fd;
   max_fd = sock_fd > max_fd ? sock_fd : max_fd;
   while(1) {
     // build the fd_set for the select
     FD_ZERO(&rfds);
     FD_SET(uavlink_serial_fd,&rfds);
     FD_SET(sock_fd,&rfds);
     selret = select(max_fd+1,&rfds,NULL,NULL,NULL);
     if (FD_ISSET(uavlink_serial_fd,&rfds)) 
       handle_serial_rx(uavlink_serial_fd,uav_link_conn);
     if (FD_ISSET(sock_fd,&rfds)) {
       printf("received udp\n");
       handle_udp_stream_rx(sock_fd,uav_link_conn);
       }
   }

   
}
