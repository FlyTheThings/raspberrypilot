

// standard include files
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

// project include files
#include <serial.h>
#include <uavlink.h>
#include <unistd.h>


#define UAVLINK_READ_BUFFER_LEN 256


static uint8_t uavlink_read_buffer[UAVLINK_READ_BUFFER_LEN];

int open_socket_uavlink(void){
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd < 0) {
		perror("Error opening socket ");
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(32000);
	int err = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if (err < 0) {
		perror ("Error binding to socket");
	}
	return sockfd;
}



int main(int argc, char**argv)
{
   int sockfd;
   int fd_uavlink_read;
   UAVLinkConnection uav_link_conn;

   //open the UDP port
   sockfd = open_socket_uavlink();

   // open the serial port
   fd_uavlink_read = serial_open();

   // configure uavlink session on serial port
   uav_link_conn = UAVLinkInitialize( (UAVLinkOutputStream) serial_write);

   int n;

   while(1) {
   n = read(fd_uavlink_read, uavlink_read_buffer, UAVLINK_READ_BUFFER_LEN);
   if (n < 0) {
     perror ("Error rx");
   }

   int i = 0;
   while (n>0) {
	   UAVLinkProcessInputStream(uav_link_conn, uavlink_read_buffer[i]);
	   i++;
	   n--;
   }
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
