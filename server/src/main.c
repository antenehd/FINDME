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

#endif

/*
000000000$QUERY
1000001$QUERY$NAME$raghu$10.19.19\n

*/

 int32 i32size = 1000;
 stRcvdMsg * testmsg = NULL;
 stRcvdMsg * testmsg1 = NULL;
 stRcvdMsg * testmsg2 = NULL;
 stRcvdMsg * testmsg3 = NULL;
 int8 msg[] = {"000000000$QUERY"};
 int8 msg1[] = {"10000001$UPDATE$NAME$raghu$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1$TIMESTAMP$12345"};
 int8 msg2[] = {"10000002$QUERY$10000001$NAME$raghu"};

 int8 msg3[] = {"1000000$UPDATE$CLIENTID$10000001$NAME$raghu$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1$TIMESTAMP$23456"};
 


 testmsg = malloc(sizeof(stRcvdMsg));
 testmsg->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg->achBuffer,msg);

 testmsg1 = malloc(sizeof(stRcvdMsg));
 testmsg1->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg1->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg1->achBuffer,msg1);

 testmsg2 = malloc(sizeof(stRcvdMsg));
 testmsg2->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg2->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg2->achBuffer,msg2);

 testmsg3 = malloc(sizeof(stRcvdMsg));
 testmsg3->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg3->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg3->achBuffer,msg3);

 if(NULL == (gpHashTable = CreateHash(i32size)))
 return -1; 
 
 HandleClientReceivedMsg(testmsg);
 HandleClientReceivedMsg(testmsg1);
 HandleClientReceivedMsg(testmsg2);
 HandleServerReceivedMsg(testmsg3);
 return 0;

}
