

// standard include files
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// project include files
#include <serial.h>
#include <uavlink.h>
#include <unistd.h>
#include <sys/select.h>

#define UAVLINK_READ_BUFFER_LEN 1


static uint8_t uavlink_read_buffer[UAVLINK_READ_BUFFER_LEN];

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

int main(int argc, char**argv)
{
   int sockfd;
   int fd_uavlink_serial;
   struct timeval tv;
   fd_set rfds;
   int selret;
   UAVLinkConnection uav_link_conn;
   printf("opening socket\n");
   
   //open the UDP port
   //sockfd = open_socket_uavlink();
   printf("opening serial ports\n");
   // open the serial port
   fd_uavlink_serial = serial_open();

   printf("configure uavlink\n");
   // configure uavlink session on serial port
   uav_link_conn = UAVLinkInitialize( (UAVLinkOutputStream) serial_write);

   while(1) {
     // build the fd_set for the select
     FD_ZERO(&rfds);
     FD_SET(fd_uavlink_serial,&rfds);
     selret = select(fd_uavlink_serial+1,&rfds,NULL,NULL,NULL);
     if (FD_ISSET(fd_uavlink_serial,&rfds)) 
       handle_serial_rx(fd_uavlink_serial,uav_link_conn);
   }

   
   /*
   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      printf("Received the following:\n");
      printf("%s",mesg);
      printf("-------------------------------------------------------\n");
   }
   */
}
