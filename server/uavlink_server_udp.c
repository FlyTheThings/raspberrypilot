#include <uavlink_server_udp.h>
#include <uavlink_server_utils.h>
#include <uavlink_server_serial.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define UDP_BFR_LEN 512
static uint8_t rx_buffer[UDP_BFR_LEN];
static uint8_t tx_buffer[UDP_BFR_LEN];

int open_socket_uavlink(int port){
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd < 0) {
		perror("Error opening socket ");
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(port);
	int err = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if (err < 0) {
		perror ("Error binding to socket");
		return -1;
	}
	return sockfd;
}

// these next two functions use last_stream_rx and stream_addr to keep track of who if anyone gets the stream forwarded to them
// this only works for one stream and one client, will have to rethink this to handle more if its needed
static uint64_t last_stream_rx = 0;
static struct sockaddr stream_addr;
static int stream_fd;
void handle_udp_stream_rx(int udp_stream_fd, UAVLinkConnection uav_link_conn) {
	int n;
	uint size = sizeof(stream_addr);
	n = recvfrom(udp_stream_fd,rx_buffer,UDP_BFR_LEN,0,&stream_addr,&size);
	UAVLinkSendStream(uav_link_conn, 1, rx_buffer, n);
	last_stream_rx = get_time_stamp();
	stream_fd = udp_stream_fd;
}
void udp_stream_tx(uint8_t *buf, uint16_t len) {
  if (get_time_stamp() < (last_stream_rx + 20000000)) {
      uint size = sizeof(stream_addr);
      //printf("udp stream tx\n");
      sendto(stream_fd,buf,len,0,&stream_addr,size);
  }
}


/*udp_uavlink packets are as follows
type - 1 byte
id - 4 bytes (lsbyte first)
data - n bytes (can optionally start with an instance)
 */
void handle_udp_uavlink_rx(int udp_link_fd, UAVLinkConnection uav_link_conn) {
  int n;
  struct sockaddr link_addr;
  uint size = sizeof(link_addr);
  uint32_t objid;
  uint8_t ptype;
  uint16_t len;
  n = recvfrom(udp_link_fd,rx_buffer,UDP_BFR_LEN,0,&link_addr,&size);
  // the udp packets are already synced so just copy out the bytes
  len = n - 5;  // len is the length of the data, not including the header
  ptype = rx_buffer[0];
  objid  = rx_buffer[1];
  objid += rx_buffer[2] << 8;
  objid += rx_buffer[3] << 16;
  objid += rx_buffer[4] << 24;
  UAVLinkSendPacket(uav_link_conn, objid, ptype, &rx_buffer[5], len);
  len = UDP_BFR_LEN;
  //printf("waiting response\n");
  if (wait_uavlink_response(uav_link_conn,rx_buffer,&len)) {
      // rx_buffer should now contain the uavlink response packet
      // to convert it to a udp_uavlink packet only some parts are copied
      tx_buffer[0] = rx_buffer[1]; // copy the message type
      tx_buffer[1] = rx_buffer[4]; // copy the obj id lsb
      tx_buffer[2] = rx_buffer[5]; 
      tx_buffer[3] = rx_buffer[6]; 
      tx_buffer[4] = rx_buffer[7]; // copy the objid msb 
      // copy the rest of the data except the checksum
      len -= 8;
      memcpy(&tx_buffer[5],&rx_buffer[8],len);
      len += 5;
      //printf("replying\n");
      sendto(udp_link_fd,&tx_buffer,len,0,&link_addr,size);
  }
}
