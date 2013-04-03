#include <udp.h>

#define UDP_MSG_BFR_LEN 512
static uint8_t mesg_buffer[UDP_MSG_BFR_LEN];

int open_socket_uavlink(void){
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
	servaddr.sin_port=htons(32000);
	int err = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if (err < 0) {
		perror ("Error binding to socket");
		return -1;
	}
	return sockfd;
}


void handle_udp_stream_rx(int udp_stream_fd, UAVLinkConnection uav_link_conn) {
       int n;
       struct sockaddr_in cliaddr;
       int len = sizeof(cliaddr);
       printf("got packet\n");
       n = recvfrom(udp_stream_fd,mesg_buffer,UDP_MSG_BFR_LEN,0,(struct sockaddr *)&cliaddr,&len);
       printf("recvfrom done\n");
       UAVLinkSendStream(uav_link_conn, 1, mesg_buffer, n);
}

