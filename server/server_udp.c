// Simple daytime client -- modified from Stevens' example at
// intro/daytimetcpcli.c

#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <stdlib.h>       // malloc
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <arpa/inet.h>  // htons, etc.
#include <sys/types.h> // required in 2.2 DNS 
#include <netdb.h>	// required in 2.2 DNS 
#include <fcntl.h> 
#include <pthread.h>
#include <netdb.h>

#define MSGMAXLEN 5
#define LISTENQ 5
#define QUEUEMAXMSG 50

int send_datagram(uint8_t type, uint32_t request_aux, int listenfd, const struct sockaddr *cli_addr6,socklen_t len){

	int n;
   char * string;
   uint32_t request;
   
   string = malloc(5);
   memcpy(string,&type,1);
   request = htonl((uint32_t) request_aux);
   memcpy((string+1),&request,8);
   //printf("Datagram to be sent %s\n",string);
   if ( (n = sendto(listenfd, string, MSGMAXLEN, 0, cli_addr6, len)) < 0) {
         perror("sendto");
         return;
   }
   printf("Datagram sent and %d bytes\n",n);
   free(string);
   return n;
}

int main(int argc, char **argv)
{
   int n;
	int listenfd;
	int server_port6;
	char * string;
	struct sockaddr_in6 cli_addr6;
	struct sockaddr_in6 local_addr6;
	socklen_t len;
	
	/* msg related stuff */
	uint8_t type;
	uint32_t request;
	char buff[80];
	memset(&buff, 0, sizeof(buff));

   /* memory and variables initialization  */ 
	string = malloc(150);
	server_port6 = 22222;
	len = sizeof(cli_addr6);
	
	/* IPV6 server configuration */
	memset(&local_addr6, 0, sizeof(local_addr6));
	local_addr6.sin6_family = AF_INET6;
	//local_addr6.sin6_addr = in6addr_any;
	local_addr6.sin6_port = htons(server_port6);
	
	
	if (inet_pton(AF_INET6, "2001:708:40:2001:0:38:3610:1", &local_addr6.sin6_addr) <= 0) {
 		fprintf(stderr, "inet_pton error for IPV6");
	return -1;
	}
	
	    
	if ((listenfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		return -1;
	}   
		
	if (bind(listenfd, (struct sockaddr *) &local_addr6,sizeof(local_addr6)) < 0) {
		perror("bind");
		return -1;
	}
	while(1){
   	
   	printf("Waiting for datagrams at %s, port %d\n",inet_ntop(AF_INET6, &local_addr6.sin6_addr,buff, sizeof(buff)),ntohs(local_addr6.sin6_port));
   	
		if ((n = recvfrom(listenfd, string, MSGMAXLEN, 0,(struct sockaddr *) &cli_addr6, &len)) < 0) {
			perror("recvfrom");
			return -1;
		}
   
      type = (uint8_t) *((char *) string);
      string++;
      request = ntohl(*((uint32_t*) string));
      string--;
      printf("Received datagram with type %d and request %lu from",type,request);
      printf("%s, port %d\n", inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)),ntohs(cli_addr6.sin6_port));
      
      send_datagram(2,2,listenfd, (struct sockaddr *) &cli_addr6,len);
     /*
	   int val;
		val = fcntl(listenfd, F_GETFL, 0);
		if (fcntl(listenfd, F_SETFL, val | O_NONBLOCK) < 0) {
		    perror("fcntl(listenfd)");
		    return;
		}
		*/ 
	  }

	free(string);
	return(0);
}
