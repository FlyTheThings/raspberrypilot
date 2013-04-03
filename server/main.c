// standard include files
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>


// project include files
#include <uavlink_server_serial.h>
#include <uavlink_server_uavlink.h>




int main(int argc, char**argv)
{
   int sock_stream_fd;
   int sock_link_fd;
   int uavlink_serial_fd;
   int max_fd;
   struct timeval tv;
   fd_set rfds;
   int selret;
   UAVLinkConnection uav_link_conn;
   printf("opening socket\n");
   
   //open the UDP port
   sock_stream_fd = open_socket_uavlink(3200);
   sock_link_fd = open_socket_uavlink(3201);
   printf("opening serial ports\n");
   // open the serial port
   uavlink_serial_fd = serial_open();

   printf("configure uavlink\n");
   // configure uavlink session on serial port
   uav_link_conn = UAVLinkInitialize( (UAVLinkOutputStream) serial_write);

   max_fd = 0;
   max_fd = uavlink_serial_fd > max_fd ? uavlink_serial_fd : max_fd;
   max_fd = sock_stream_fd > max_fd ? sock_stream_fd : max_fd;
   max_fd = sock_link_fd > max_fd ? sock_link_fd : max_fd;
   while(1) {
     // build the fd_set for the select
     FD_ZERO(&rfds);
     FD_SET(uavlink_serial_fd,&rfds);
     FD_SET(sock_stream_fd,&rfds);
	 FD_SET(sock_link_fd,&rfds)
     selret = select(max_fd+1,&rfds,NULL,NULL,NULL);
     if (FD_ISSET(uavlink_serial_fd,&rfds)) 
       handle_serial_rx(uavlink_serial_fd,uav_link_conn);
     if (FD_ISSET(sock_stream_fd,&rfds)) {
       printf("received udp\n");
       handle_udp_stream_rx(sock_stream_fd,uav_link_conn);
       }
   }

   
}
