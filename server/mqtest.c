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

#define MSGLENGHT 5
#define LISTENQ 5

#define QUEUEMAXMSG 50

int main(int argc, char **argv)
{
	mqd_t mq;
	struct mq_attr attr;
	char buffer[MAX_SIZE + 1];
	
	/* initialize the queue attributes */
	attr.mq_flags = 0;
	attr.mq_maxmsg = QUEUEMAXMSG;
	attr.mq_msgsize = MSGLENGHT;
	attr.mq_curmsgs = 0;
	int flags = O_RDWR | O_CREAT;
	  	 mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	
	/* create the message queue */
	mq = mq_open("/QUEUE_UPD_TO_DATA", flags, mode, &attr);
	if( (mqd_t)-1 == mq){
	 	perror("mqopen error");
	}
	mq_send(mq, buffer, MAX_SIZE, 0)


	free(string);
	return(0);
}
