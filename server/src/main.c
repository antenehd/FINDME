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


HashTable_t  *gpHashTable = NULL;
int main()
{


#if 0
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
/*
HandleClientReceivedMsg(NULL);
HandleServerReceivedMsg(NULL);
*/
pthread_join(sync_thread , NULL);

 int32 i32size = 1000;
 stRcvdMsg testmsg = {0};
 int8 msg[] = {"000000000$QUERY"}
 testmsg->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg->achBuffer = malloc(MAX_MSG_LEN); 
 memcpy(testmsg->achBuffer,);
 HandleClientReceivedMsg(NULL);
 if(NULL == (gpHashTable = CreateHash(i32size)))
 return -1; 
 



#endif
return 0;

}
