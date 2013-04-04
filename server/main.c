// standard include files
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>


// project include files
#include <uavlink_server_serial.h>
#include <uavlink_server_udp.h>




void stream_forwarder(uint32_t id, uint8_t * buf, uint16_t buf_len){
  //only forwards stream 1, to the udp stream 
  if (id == 1) {
    udp_stream_tx(buf, buf_len);    
  }
}

int main(int argc, char**argv)
{
  int sock_stream_fd;
  int sock_link_fd;
  int uavlink_serial_fd;

   int max_fd;
   fd_set rfds;
   UAVLinkConnection uav_link_conn;
   
   
   //open the UDP port
   sock_stream_fd = open_socket_uavlink(32000);
   sock_link_fd = open_socket_uavlink(32001);
   
   // open the serial port
   uavlink_serial_fd = serial_open();

   
   // configure uavlink session on serial port
   uav_link_conn = UAVLinkInitialize( (UAVLinkOutputStream) serial_write);
   UAVLinkSetStreamForwarder(uav_link_conn, stream_forwarder);



   max_fd = 0;
   max_fd = uavlink_serial_fd > max_fd ? uavlink_serial_fd : max_fd;
   max_fd = sock_stream_fd > max_fd ? sock_stream_fd : max_fd;
   max_fd = sock_link_fd > max_fd ? sock_link_fd : max_fd;
   while(1) {
     // build the fd_set for the select
     FD_ZERO(&rfds);
     FD_SET(uavlink_serial_fd,&rfds);
     FD_SET(sock_stream_fd,&rfds);
     FD_SET(sock_link_fd,&rfds);
     select(max_fd+1,&rfds,NULL,NULL,NULL);
     if (FD_ISSET(uavlink_serial_fd,&rfds)) 
       handle_serial_rx(uavlink_serial_fd,uav_link_conn);
     if (FD_ISSET(sock_stream_fd,&rfds)) {
       handle_udp_stream_rx(sock_stream_fd,uav_link_conn);
       }
     if (FD_ISSET(sock_link_fd,&rfds)) {
       handle_udp_uavlink_rx(sock_link_fd,uav_link_conn);
       }
   }

   
}
