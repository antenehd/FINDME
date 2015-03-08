#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>

#include "data_structure.h"
#include "proto_types.h"


int main()
{

/*int8 achTestArray[] = {"1000001$QUERY$NAME$raghu$10.19.19\n"};*/ 

/*TODO message queues and sockets*/
pthread_t prcs_thread = {0};
pthread_t sync_thread = {0};

/* Create the threads*/
if(pthread_create(&prcs_thread , NULL , &ProcessThreadStart, NULL) != 0){
	perror("Pthread_Create");
        return -1;
	}

if(pthread_create(&sync_thread, NULL , &SyncThreadStart, NULL) != 0){
	perror("Pthread_Create");
        return -1;
	}

HandleClientReceivedMsg(NULL);
HandleServerReceivedMsg(NULL);
pthread_join(sync_thread , NULL);
return 0;

}
