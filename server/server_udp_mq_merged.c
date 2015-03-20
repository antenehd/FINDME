/* TO BE COMPILED WITH -LTR */
/* gcc -o server server_udp_mq_merged.c -lrt */
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
#include <mqueue.h>
#include <errno.h>

/* ipv6 server details */
#define LISTENQ 5

/* message queue details */
#define MQ_NAME       "/testmsg"
#define MAX_MSG         100
#define MQ_MODE 		0777

/* message details - to be sync with data_strucure.h */
#define MAX_MSG_LEN 1024

/* structure to fill with msg form socket and send to msq*/
/* no pointers to be passed through msg queue, they have no meaning in the other process memory */
typedef struct { 
	struct sockaddr_in6 strcvd_addr; /* in data structure it's not ipv6 */
	char achBuffer[MAX_MSG_LEN];
} msg_t;
 
int check_datagram(datagram){
	/* do I need to check for correct end (jump of line) of datagram? in case datagram is bigger than MAX_MSG_LEN what we do? */
	return 0;
}
int send_to_mq( mqd_t queue, struct sockaddr_in6 cli_addr6, unsigned prio, char * packet){
  
  size_t mlen;
  msg_t * mess;
  char buff[80]; 	/* used by inet_ntop function */

  /* we fill the msg_t structure */
  mlen = sizeof(cli_addr6) + strlen(packet);
  if (mlen > 2 * MAX_MSG_LEN){
	  /* mqstat.mq_msgsize = 2 * MAX_MSG_LEN; */
	  printf("ERROR : The message to be put to the mq is too long\n");
	  return -1;
  }
  mess = (msg_t *) malloc(mlen);
  //printf("SEND TO MQ - Size of cli_addr is %d, strlen of packet is %d, mlen %d\n",sizeof(cli_addr6),strlen(packet),mlen);
  
  mess->strcvd_addr = cli_addr6;
  memcpy(mess->achBuffer, packet, strlen(packet));
  printf("MQ: INFO: ArchBiffer equals <%s> and addr equals <%s> and port <%d>\n",mess->achBuffer,inet_ntop(AF_INET6, &mess->strcvd_addr,buff, sizeof(buff)),ntohs(mess->strcvd_addr.sin6_port));
  
  if(mq_send(queue, (char *) mess, mlen, prio) < 0)
  {
	if (errno == EINTR)
	   /* the queue was interrupted by a signal */
		printf("WARNING : EINTR - mq_send was interrupted by signal\n");
		return -1;

   if (errno == EMSGSIZE) {
		/* buffer that receives messages is smaller that the mq_msgsize */
		printf("ERROR : EMSGSIZE - mq_send, msg_len was greater than the mq_msgsize attribute of the message queue\n");
		perror("mq_send error");	
		return -1;
   }
   else{
	    printf("ERROR : mq_send\n");
		perror("mq_send error");	
		return -1;
   }
  }
  else printf("MQ: INFO: Message <%s> successfully introduced to mq with mlen <%d>\n",packet,mlen);
  /* !!!  HACK HACK HACK HACK  !!!!*/
  mq_send(queue, (char *) mess, mlen, prio);
  free(mess);
  
  return 0;
}

int main(int argc, char **argv)
{
    int n,i;
	/* ipv6 server variables */
	int listenfd, server_port6;
	struct sockaddr_in6 local_addr6,cli_addr6;
	socklen_t len;
	char buff[80];
	char *datagram, *server_ip6;
	
	/* mq related stuff */
	mqd_t queue;
	unsigned prio = 1;
	struct mq_attr mqstat;

   /* memory and variables initialization  */ 
	memset(&buff, 0, sizeof(buff));
	//server_ip6 = "2001:708:40:2001:0:38:3610:1"; /* nwprog1.netlab.hut.fi */
	server_ip6 = "::ffff:195.148.124.76"; /*  nwprog1.netlab.hut.fi */
	server_port6 = 22222;
	len = sizeof(cli_addr6);
	datagram = malloc(MAX_MSG_LEN);
	
	/* initialization of IPV6 server */
	memset(&local_addr6, 0, sizeof(local_addr6));
	local_addr6.sin6_family = AF_INET6;
	local_addr6.sin6_port = htons(server_port6);
	if (inet_pton(AF_INET6, server_ip6, &local_addr6.sin6_addr) <= 0) {
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
	/* server ready */
	
	/* initialization of message queue */
	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_maxmsg = MAX_MSG;
	mqstat.mq_msgsize = 2 * MAX_MSG_LEN; /* Controlled in send_to_mq() */
	mqstat.mq_flags = 0;
    queue = mq_open(MQ_NAME,O_CREAT|O_WRONLY, MQ_MODE, &mqstat);
    if(queue == -1) {
		perror("Failed to open message queue");
		return 0;
    }  
	/* message queue ready */
	
	while(1){
		printf("UDPS: INFO: Waiting for datagrams at %s, port %d\n",inet_ntop(AF_INET6, &local_addr6.sin6_addr,buff, sizeof(buff)),ntohs(local_addr6.sin6_port));
		if ((n = recvfrom(listenfd, datagram, MAX_MSG_LEN, 0,(struct sockaddr *) &cli_addr6, &len)) < 0) {
			printf("ERROR: recvfrom : number of bytes received %d\n",n);
			perror("recvfrom");
			return -1;
		}
		printf("UDPS: INFO: Received datagrams (%d bytes) from %s, port %d\n",n,inet_ntop(AF_INET6, &local_addr6.sin6_addr,buff, sizeof(buff)),ntohs(local_addr6.sin6_port));
		if(check_datagram(datagram) < 0){
			printf("UDPS: ERROR: check_datagram : packet discarded\n");
		}
		else{
			if(send_to_mq(queue,cli_addr6,prio,datagram) < 0 ){
				printf("ERROR: send_to_mq : packet can not be put in the mq\n");
			}
		}
	  }

	free(datagram);
	close (listenfd);
	return(0);
}
