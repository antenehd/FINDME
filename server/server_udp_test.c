/* gcc -o client server_udp_test.c */
/* EXECUTED IN NWPROG1 SERVER TO MATCH IP */

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

#define MSGLENGHT 5
#define LISTENQ 5

/* message details - to be sync with data_strucure.h */
#define MAX_MSG_LEN 1024

int send4_datagram(char * msg, int listenfd,struct sockaddr_in cli_addr ,socklen_t len){

	int n;
	char buff[80];
   
   //printf("Datagram to be sent %s\n",string);
   if ( (n = sendto(listenfd, msg, MAX_MSG_LEN , 0, (const struct sockaddr *) &cli_addr, len)) < 0) {
         perror("sendto");
         return;
   }
   printf("IPV4 Datagram sent (%d bytes) ",n);
   printf("to %s, port %d\n", inet_ntop(AF_INET,  &cli_addr.sin_addr,buff, sizeof(buff)),ntohs(cli_addr.sin_port));
   return n;
}

int send6_datagram(char * msg, int listenfd, struct sockaddr_in6 cli_addr6 ,socklen_t len){

	int n;
	char buff[80];
   
   //printf("Datagram to be sent %s\n",string);
   if ( (n = sendto(listenfd, msg, MAX_MSG_LEN , 0, (const struct sockaddr * ) &cli_addr6, len)) < 0) {
         perror("sendto");
         return;
   }
   printf("IPV6 Datagram sent (%d bytes) ",n);
   printf("to %s, port %d\n", inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)),ntohs(cli_addr6.sin6_port));
   return n;
}
int send6_datagram_longmsg(char * msg, int listenfd, struct sockaddr_in6 cli_addr6 ,socklen_t len){

	int n;
	char buff[80];
   
   //printf("Datagram to be sent %s\n",string);
   if ( (n = sendto(listenfd, msg, 2*MAX_MSG_LEN , 0, (const struct sockaddr * ) &cli_addr6, len)) < 0) {
         perror("sendto");
         return;
   }
   printf("IPV6 Datagram sent (%d bytes) ",n);
   printf("to %s, port %d\n", inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)),ntohs(cli_addr6.sin6_port));
   return n;
}

int main(int argc, char **argv)
{
   int n,i,client_port;
	int sockfd6,sockfd4,listenfd;
	int server_port6;
	char * string, *client_ipv4, *client_ipv6;
	struct sockaddr_in cli_addr,local_addr;
	struct sockaddr_in6 cli_addr6,local_addr6;
	socklen_t len,len4;
	
	/* msg related stuff */
	char buff[80];
	memset(&buff, 0, sizeof(buff));

   /* memory and variables initialization  */ 
	string = malloc(MAX_MSG_LEN);
	memset(&buff, 0, sizeof(buff));
	//server_port6 = 44444;
	len = sizeof(cli_addr6);
	len4 = sizeof(cli_addr);
	client_ipv4 = "195.148.124.76";
	client_ipv6 = "::ffff:195.148.124.76";
	client_port = 22222;
	
	/* OLD 
	/* IPV6 configuration 
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
	ready to listen to datagram but nobody knows we are here */
	
	/* IPV6 client configuration */	
	if ( (sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		return -1;
	 }
	memset(&cli_addr6, 0, sizeof(cli_addr6));
	cli_addr6.sin6_family = AF_INET6;
	cli_addr6.sin6_port = htons(client_port);
	
	if (inet_pton(AF_INET6, client_ipv6, &cli_addr6.sin6_addr) <= 0) {
 		fprintf(stderr, "inet_pton error for IPV6");
	return -1;
	}
	/* end of IPV6 client configuration */
	
	/* IPV4 client configuration */	
	if ( (sockfd4 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		return -1;
	 }
	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(client_port);
	
	if (inet_pton(AF_INET, client_ipv4, &cli_addr.sin_addr) <= 0) {
 		fprintf(stderr, "inet_pton error for IPV4");
	return -1;
	}
	/* BATTERY TEST  */
	
	/*  TEST 1, IPv4 and IPv6 message overflow */
	
	///*
	for (i = 0; i < 1000; i ++){
		//printf("Sending datagram to %s, port %d\n", inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)),ntohs(cli_addr6.sin6_port));	
		sprintf(string,"Test msg %d from %s",i,inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)));
		send6_datagram(string,sockfd6, cli_addr6,len);
		
		sprintf(string,"Test msg %d from %s",i,inet_ntop(AF_INET, &cli_addr.sin_addr,buff, sizeof(buff)));
		send4_datagram(string,sockfd4, cli_addr,len4);
	}
	
	//*/
	/* TEST 2, Message over MAX_MSG_LEN
	At this point this case is not yet operative because msg_receive only reads from MAX_MSG_LEN to MAX_MSG_LEN and end of msg is not checked.
	*/
	
	/*
	string = malloc(2*MAX_MSG_LEN);
	memset(string, 0, sizeof(string));
	send6_datagram_longmsg(string,sockfd6, cli_addr6,len);
	
	*/
	
	
	/* OLD
	while(1){
   	
   	printf("Waiting for datagrams at %s, port %d\n",inet_ntop(AF_INET6, &local_addr6.sin6_addr,buff, sizeof(buff)),ntohs(local_addr6.sin6_port));
		if ((n = recvfrom(listenfd, string, MSGLENGHT, 0,(struct sockaddr *) &cli_addr6, &len)) < 0) {
			perror("recvfrom");
			return -1;
		}
   
      type = (uint8_t) *((char *) string);
      string++;
      request = ntohl(*((uint32_t*) string));
      string--;
      printf("Received datagram with type %d and request %lu from",type,request);
      printf("%s, port %d\n", inet_ntop(AF_INET6, &cli_addr6.sin6_addr,buff, sizeof(buff)),ntohs(cli_addr6.sin6_port));
      
      */
     /*
	   int val;
		val = fcntl(listenfd, F_GETFL, 0);
		if (fcntl(listenfd, F_SETFL, val | O_NONBLOCK) < 0) {
		    perror("fcntl(listenfd)");
		    return;
		}
		*/ 
	  //}

	free(string);
	close(sockfd6);
	close(sockfd4);
	return(0);
}
