#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <errno.h>

#include <arpa/inet.h>   // inet_pton, ...
#include <netdb.h>
#include <netinet/in.h>  // defines sockaddr_in

/* message queue details */
#define MQ_NAME       "/testmsg"
#define MAX_MSG         100
#define MQ_MODE 	0777

/* message details - to be sync with data_strucure.h */
#define MAX_MSG_LEN 1024

	/* message details - to be sync with data_strucure.h */
	/* no pointers to be passed through msg queue, they have no meaning in the other process memory */
	
  typedef struct { 
	struct sockaddr_in6 strcvd_addr; /* in data structure it's not ipv6 */
	char achBuffer[MAX_MSG_LEN];
  } msg_t;
  
int send_to_mq( mqd_t queue, struct sockaddr_in6 cli_addr6, unsigned prio, char * packet){
  
  size_t mlen;
  msg_t * mess;
  char buff[80]; 	/* used by inet_ntop function */

 
  /* we fill the msg_t structure */
  //mlen = sizeof(*cli_addr6) + strlen(packet);
  mlen = sizeof(cli_addr6) + strlen(packet);
  if (mlen > 2 * MAX_MSG_LEN){
	  /* mqstat.mq_msgsize = 2 * MAX_MSG_LEN; */
	  printf("ERROR : The message to be put to the mq is too long\n");
	  return -1;
  }
  mess = (msg_t *) malloc(mlen);
  printf(" SEND TO MQ - Size of cli_addr is %d, strlen of packet is %d, mlen %d\n",sizeof(cli_addr6),strlen(packet),mlen);
  
  mess->strcvd_addr = cli_addr6;
  //memcpy(&mess->strcvd_addr, cli_addr6, sizeof(*cli_addr6));
  memcpy(mess->achBuffer, packet, strlen(packet));
  printf("INFO: ArchBiffer equals <%s> and addr equals <%s> and port <%d>\n",mess->achBuffer,inet_ntop(AF_INET6, &mess->strcvd_addr,buff, sizeof(buff)),ntohs(mess->strcvd_addr.sin6_port));
  
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
  else printf("INFO: Message <%s> successfully introduced to mq with mlen <%d>\n",packet,mlen);
  /* !!!  HACK HACK HACK HACK  !!!!*/
  mq_send(queue, (char *) mess, mlen, prio);
  free(mess);
  
  return 0;
}

int main(int argc, char *argv[]) {
	
	unsigned prio = 1;
    mqd_t queue;
	struct sockaddr_in6 cli_addr6;
	struct mq_attr mqstat;
	//printf("sizeof no pointer %d, size of pointer %d",sizeof(cli_addr6),sizeof(*cli_addr62));
	
	memset(&cli_addr6, 0, sizeof(cli_addr6));
	cli_addr6.sin6_family = AF_INET6;
	cli_addr6.sin6_port = htons(22223);
	if (inet_pton(AF_INET6, "2001:708:1140:2001:0000:38:3610:0001", &cli_addr6.sin6_addr) <= 0) {
 		fprintf(stderr, "inet_pton error for IPV6");
	return -1;
	}
	char packet[MAX_MSG_LEN];
	char * tmp;
	tmp = malloc(MAX_MSG_LEN);
	//char * tmp = "I'm a test message";
	//memcpy(packet, tmp, strlen(tmp));
	//memcpy(packet+strlen(tmp), "\0", 1);
	//printf("Value of packet is %s,%d,%d,%d\n",packet,strlen(packet),strlen(tmp),sizeof(packet));
	//printf("*packet = %p\n", (void *)packet);
	//printf("*tmp = %p\n", (void *)tmp);
	
  memset(&mqstat, 0, sizeof(mqstat));
  mqstat.mq_maxmsg = MAX_MSG;
  mqstat.mq_msgsize = 2 * MAX_MSG_LEN;
  mqstat.mq_flags = 0;
  
  queue = mq_open(MQ_NAME,O_CREAT|O_WRONLY, MQ_MODE, &mqstat);
  if(queue == -1) {
    perror("Failed to open message queue");
    return 0;
  }  
	
  int i;
  for (i = 0; i < 10000; i++){
		//sleep(0.001);
	  	sprintf(tmp, "I'm a test message %d",i);
		//printf("Value of tmp %s\n",tmp);
		memcpy(packet, tmp, strlen(tmp));
		memcpy(packet+strlen(tmp), "\0", 1);
		packet-strlen(tmp);
	
	  if(send_to_mq(queue,cli_addr6,prio,packet) < 0 ){
		  printf("ERROR : sent_to_mq failed \n");
	  }
	  
  }
  exit(0);
}
