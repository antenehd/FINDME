#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <arpa/inet.h>   // inet_pton, ...
#include <netdb.h>
#include <netinet/in.h>  // defines sockaddr_in

/* message queue details */
#define MQ_NAME       "/testmsg"
#define MAX_MSG         100

/* message details - to be sync with data_strucure.h */
#define MAX_MSG_LEN 1024  

int main(int argc, char *argv[]) {

  typedef struct { 
	struct sockaddr_in6 strcvd_addr; /* in data structure it's not ipv6 */
	char achBuffer[MAX_MSG_LEN];
  } msg_t;

  size_t msize;     /* amount of reserved space */
  msg_t * mess;
  mqd_t queue;
  unsigned prio;
  char buff[80]; 	/* used by inet_ntop function */
  int n;
  
  	/* we initialize to get size */
	struct sockaddr_in6 cli_addr6;
	memset(&cli_addr6, 0, sizeof(cli_addr6));
	
  /* we prepare to read up to MAX_MSG_LEN */
  msize = 2 * MAX_MSG_LEN + 1;
  mess = (msg_t *) malloc(msize);
  
  	queue = mq_open(MQ_NAME,O_RDONLY);
  if(queue == -1) {
    perror("ERROR : Failed to open message queue");
    return 0;
  }
  
  printf("Process %d ready to receive messages. msq_t %ld, and msize <%d> >\n",(int)getpid(),(long) queue,msize);

  while(1){
	  memset(mess , 0, msize);
	 /* Get messages for this process ! */
	 if ((n = mq_receive(queue, (char *) mess, msize, &prio)) < 0 ) {
	   if (errno == EINTR)
		   /* the queue was interrupted by a signal */
			printf("WARNING : EINTR - mq_receive was interrupted by signal\n");;

	   if (errno == EMSGSIZE) {
			/* buffer that receives messages is smaller that the mq_msgsize */
			printf("ERROR : EMSGSIZE - mq_receive, msg_len was less than the mq_msgsize attribute of the message queue\n");
			perror("mq_receive error");	
	   }
	   else{
			printf("ERROR : mq_receive\n");
			perror("mq_receive error");	
			return -1;
	   }
	 }
	 printf("Message <%s> prio <%u> and ip address <%s>,with port <%d> with mlen <%d> and msize <%d>\n", mess->achBuffer, prio, inet_ntop(AF_INET6, &mess->strcvd_addr,buff, sizeof(buff)),ntohs(mess->strcvd_addr.sin6_port),n,msize);  
  }
  free(mess);

  exit(0);
}

