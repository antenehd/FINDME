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
#include <netdb.h>      
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>
#include <mqueue.h>
#include <errno.h>

#include "data_structure.h"
#include "proto_types.h"

pthread_mutex_t stRecMutex = {{0}};
HashTable_t  *gpHashTable = NULL;
int32 gUDPCliSockFD = 0;
int32 gUDPServSockFD = 0;
mqd_t gMsgQID = 0;
FILE * fpLog = NULL;

extern stConfigFileItems gstConfigs;
int32 CreateUDPSock(uint32 PortNum)
{
   /* ipv6 server variables */
   int listenfd = 0;
   struct sockaddr_in6 local_addr6 = {0};

   if ((listenfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
           perror("socket error");
           return -1;
   }
   /* initialization of IPV6 server */
   memset(&local_addr6, 0, sizeof(local_addr6));
   local_addr6.sin6_family = AF_INET6;
   local_addr6.sin6_port = htons(PortNum);
   local_addr6.sin6_addr = in6addr_any;
/*
   if (inet_pton(AF_INET6, server_ip6, &local_addr6.sin6_addr) <= 0) {
           fprintf(stderr, "inet_pton error for IPV6");
           return -1;
   }
*/
      if (bind(listenfd, (struct sockaddr *) &local_addr6,sizeof(local_addr6)) < 0) {
           perror("bind");
           return -1;
   }

return listenfd;

}

int32 CreateMsgQueue()
{

  /* mq related stuff */
  mqd_t queue = 0;
  struct mq_attr mqstat;

  /* initialization of message queue */
  memset(&mqstat, 0, sizeof(mqstat));
  mqstat.mq_maxmsg = MAX_MSG;
  mqstat.mq_msgsize = MAX_MSG_LEN; /* Controlled in send_to_mq() */
  mqstat.mq_flags = 0;
  
   queue = mq_open(MQ_NAME,O_CREAT|O_RDWR, MQ_MODE, &mqstat);
   perror("Message queue");
   printf("Msgq ID = %d\n", queue);
 return queue;
}


int main()
{

#if 1
 int32 i32size = 1000;
 stRcvdMsg * pstNewMsg = NULL;
int8*  pachTestArray = NULL; 
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;
uint64 ui64ID = 0;
uint64 ui64ServID = 0;
uint32 priority = 1;
pthread_t prcs_thread = {0};

/*Create Log File*/
 if(NULL == (fpLog = fopen(LOG_FILE,"a")))
 {
   printf("ERROR: Log file cannot be created\n");
   return -1;
 }
 FINDME_LOG("INFO :Server Started\n");

if(NULL == (gpHashTable = CreateHash(i32size)))
{
    FINDME_LOG("ERROR :Hash creation failed\n");
    return -1;
}
/*Create socket*/
if(0 > (gUDPCliSockFD = CreateUDPSock(CLI_PORT_NUM)))
{
    /*socket creation failed*/
    FINDME_LOG("ERROR :Client socket creation failed\n");
    return -1;
}

if(0 > (gUDPServSockFD = CreateUDPSock(SERV_PORT_NUM)))
{
    /*socket creation failed*/
    FINDME_LOG("ERROR :Server socket creation failed\n");
    return -1;
}

FINDME_LOG("INFO :Sockets created\n");
/*Create socket*/
if(0 > (gMsgQID = CreateMsgQueue()))
{
    /*message creation failed*/
    FINDME_LOG("ERROR :Message queue  creation failed\n");
    return -1;
}

FINDME_LOG("INFO :Message Queue Created\n");
if(0 > (readConfigFile()))
{
    FINDME_LOG("ERROR :Reading configuration failed\n");
     return -1;
}

FINDME_LOG("INFO :Reading Configuration successful\n");

ui64ServID = strtol(MSERVERID,NULL,0);
if(ui64ServID == gstConfigs.ui64ServID)
{
     /*New server, so request for an ID*/
     RequestServID();
}

/* Create the threads*/
if(pthread_create(&prcs_thread , NULL , &ProcessThreadStart, NULL) != 0){
        FINDME_LOG("ERROR :Thread Creation unsuccessful\n");
        return -1;
	}

FINDME_LOG("INFO :Thread Creation successful\n");
fflush(fpLog);
while(1)
{
   if(NULL != (pstNewMsg = CreateMsg()))
   {
      if(-1  ==  mq_receive(gMsgQID ,(char *) pstNewMsg, MAX_MSG_LEN+1, &priority))
      {
         perror("Mq receive");
         continue;
      }
   
       pachTestArray = pstNewMsg->achBuffer; 
      /*Check for the ID*/
      pi8Token = strtok_r(pachTestArray ,DELIMITER , &pi8SavePtr);
      ui64ID = strtol(pi8Token,NULL,0);

      if(0 == ui64ID)
      {
         /*If it is a new client*/
         AssignIDToCli(pstNewMsg);
      }
      else if(0 == isclient(ui64ID))
      {
         /*process message from client*/
         HandleClientReceivedMsg(pstNewMsg , pi8SavePtr);
      } 
      else if(0 == isServer(ui64ID))
      {
         /*process message from server*/
         HandleServerReceivedMsg(pstNewMsg , pi8SavePtr);
      }
      else
      {
         /*Message from unknown entity*/
         FINDME_LOG("ERROR :Unknown message received\n");
      }

   } 
}

#endif
return 0;
}




/*
000000000$QUERY
1000001$QUERY$NAME$raghu$10.19.19\n

*/
#if 0
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
#endif

