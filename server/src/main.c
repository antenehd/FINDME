#include<stdio.h>
#include<signal.h>
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

pthread_mutex_t stRecMutex = {{0}};
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
 stRcvdMsg * testmsg4 = NULL;
 stRcvdMsg * testmsg5 = NULL;
 int8 msg[] = {"000000000$QUERY"};
 int8 msg1[] = {"10000001$UPDATE$NAME$varun$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1$TIMESTAMP$12345"};
 int8 msg2[] = {"10000002$QUERY$10000001$NAME$raghu"};

 int8 msg3[] = {"1000000$UPDATE$CLIENTID$10000001$NAME$raghu$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1$TIMESTAMP$23456"};
 int8 msg4[] = {"1000000$NEW$CLIENTID$10000001$NAME$varun$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1"}; 
 int8 msg5[] = {"1000000$NEW$CLIENTID$10000005$NAME$varun$EMAIL$raghunfs@gmail.com$ADDRESS$Espoo$LOCATION$192.168.0.1"}; 
 pthread_mutex_init(&stRecMutex,NULL);

 if(signal(SIGINT,sighandler_SIGINT) == SIG_ERR)
     FINDME_LOG("SIGINT cant be handled\n");

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

 testmsg4 = malloc(sizeof(stRcvdMsg));
 testmsg4->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg4->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg4->achBuffer,msg4);

 testmsg5 = malloc(sizeof(stRcvdMsg));
 testmsg5->strcvd_addr = malloc(sizeof(struct sockaddr_in)); 
 testmsg5->achBuffer = malloc(MAX_MSG_LEN); 
 strcpy(testmsg5->achBuffer,msg5);

 if(NULL == (gpHashTable = CreateHash(i32size)))
 return -1; 
 
 printf("\nIn line %d\n",__LINE__);
 HandleClientReceivedMsg(testmsg);
 printf("\nIn line %d\n",__LINE__);
 HandleClientReceivedMsg(testmsg1);
 printf("\nIn line %d\n",__LINE__);
 HandleClientReceivedMsg(testmsg2);
 printf("\nIn line %d\n",__LINE__);
 HandleServerReceivedMsg(testmsg3);
 printf("\nIn line %d\n",__LINE__);
 HandleServerReceivedMsg(testmsg4);
 printf("\nIn line %d\n",__LINE__);
 HandleServerReceivedMsg(testmsg5);
 printf("\nIn line %d\n",__LINE__);

  PrintAllRecord();
  sighandler_SIGINT(); 
 /*kill(getpid(), SIGINT);*/
 return 0;

}
