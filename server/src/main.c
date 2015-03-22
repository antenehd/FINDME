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
/*TODO : 
 1)sockname
 2)select
 3) writeback to conf file
*/

extern stConfigFileItems gstConfigs;
int32 CreateUDPSock(uint32 PortNum)
{
   /* ipv6 server variables */
   int listenfd = 0;
   struct sockaddr_in6 local_addr6 = {0};
/*
   char server_ip6[MAX_LINE_LENGTH] = {0};
   struct sockaddr_in alt_conn = {0};
   socklen_t size = sizeof(alt_conn);
   char my_addr[100] = {0};
  
   if (getsockname(listenfd, (struct sockaddr *) &alt_conn, &size) < 0){
    perror("getsockname");
    return -1;
    }
    if (!inet_ntop(AF_INET, &alt_conn.sin_addr, my_addr, sizeof(alt_conn))) 
    {
         perror("inet_ntop");
         return -1;
    }

    strcat(server_ip6 , "::ffff:");
    strcat(server_ip6 , my_addr);
 */
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
  mqstat.mq_msgsize = 2 * MAX_MSG_LEN; /* Controlled in send_to_mq() */
  mqstat.mq_flags = 0;
  
   queue = mq_open(MQ_NAME,O_CREAT|O_WRONLY, MQ_MODE, &mqstat);

 return queue;
}


int main()
{

#if 1
 stRcvdMsg * pstNewMsg = NULL;
int8*  pachTestArray = NULL; 
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;
uint64 ui64ID = 0;
uint64 ui64ServID = 0;
uint32 priority = 1;
pthread_t prcs_thread = {0};

/*Create socket*/

if(0 > (gUDPCliSockFD = CreateUDPSock(CLI_PORT_NUM)))
{
    /*socket creation failed*/
    return -1;
}
if(0 > (gUDPServSockFD = CreateUDPSock(SERV_PORT_NUM)))
{
    /*socket creation failed*/
    return -1;
}
/*Create socket*/

if(0 > (gMsgQID = CreateMsgQueue()))
{
    /*message creation failed*/
    return -1;
}

if(0 > (readConfigFile()))
{
     return -1;
}

ui64ServID = strtol(MSERVERID,NULL,0);
if(ui64ServID == gstConfigs.ui64ServID)
{
     RequestServID();
}

/* Create the threads*/
if(pthread_create(&prcs_thread , NULL , &ProcessThreadStart, NULL) != 0){
	perror("Pthread_Create");
        return -1;
	}


while(1)
{
   if(-1  ==  mq_receive(gMsgQID ,(char *) pstNewMsg, sizeof(stRcvdMsg), &priority))
   {
       /*TODO : LOG*/
        continue;
   }
   
   pachTestArray = pstNewMsg->achBuffer; 
     /*Check for the ID*/
   pi8Token = strtok_r(pachTestArray ,DELIMITER , &pi8SavePtr);
   ui64ID = strtol(pi8Token,NULL,0);

   if(0 == ui64ID)
   {
       AssignIDToCli(pstNewMsg);
   }
   else if(0 == isclient(ui64ID))
   {
       HandleClientReceivedMsg(pstNewMsg , pi8SavePtr);
   } 
   else if(0 == isServer(ui64ID))
   {
       HandleServerReceivedMsg(pstNewMsg , pi8SavePtr);
   }
   else
   {
      printf("Error: unknown Message\n");
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
#endif

