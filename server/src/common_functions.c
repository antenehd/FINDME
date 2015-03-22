#include<stdio.h>
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

extern mqd_t gMsgQID;
extern int32 gUDPCliSockFD;
extern int32 gUDPServSockFD;

stConfigFileItems gstConfigs;

void ChangeServID()
{

  FILE * fpConfig = NULL;
  int8  achReadBuff[MAX_LINE_LENGTH] = {0};
  int8  achWriteBuff[MAX_LINE_LENGTH] = {0};
  int8 * pi8SavePtr = NULL;
  int8 * pi8Token = NULL;
  int8 * pi8Value = NULL;
  int32 i32Totlen = 0;
  int32 i32len = 0;

  if( NULL != (fpConfig = fopen(CONFIG_FILE ,"w")))
  {
     while(!feof(fpConfig))
     {
        if(NULL != fgets(achReadBuff,MAX_LINE_LENGTH,fpConfig))
        {
            pi8Token = strtok_r(achReadBuff ,"=" , &pi8SavePtr);                       
            i32len = strlen(achReadBuff);
            i32Totlen += i32len;

            if(0 == strcmp("SERVER_ID", pi8Token))
            {
               fseek(fpConfig,(i32Totlen - i32len),SEEK_SET);
               snprintf(achWriteBuff ,MAX_LINE_LENGTH,"SERVER_ID=%ld\n",gstConfigs.ui64ServID );
               fwrite(achWriteBuff , strlen(achWriteBuff),1,fpConfig);
               break;
            }
         }
      }
      fclose(fpConfig);
   }
}

int32 readConfigFile()
{
  FILE * fpConfig = NULL;
  int32 i32retval = -1;
  int8  achReadBuff[MAX_LINE_LENGTH] = {0};
  int8 * pi8SavePtr = NULL;
  int8 * pi8Token = NULL;
  int8 * pi8Value = NULL;

  if( NULL != (fpConfig = fopen(CONFIG_FILE ,"r")))
  {
     while(!feof(fpConfig))
     {
        if(NULL != fgets(achReadBuff,MAX_LINE_LENGTH,fpConfig))
        {
            pi8Token = strtok_r(achReadBuff ,"=" , &pi8SavePtr);                       
            pi8Value = strtok_r(NULL ,"=" , &pi8SavePtr);
            if(0 == strcmp("MASTER_IP", pi8Token))
            {
               memcpy(gstConfigs.achServerIP, pi8Value, strlen(pi8Value));
            }

            if(0 == strcmp("SERVER_ID", pi8Token))
            {
               gstConfigs.ui64ServID = strtol(pi8Value,NULL,0);
            }

            if(0 == strcmp("PORT", pi8Token))
            {
               gstConfigs.ui32Port = strtol(pi8Value,NULL,0);
            }                        
        }
     }
     i32retval = 0;
     fclose(fpConfig);
  } 
  return i32retval;
}

void MapV4toV6( int8 * achAddr)
{

 int8 achbuff[] = ":ffff:";
 
 strcat(achAddr,achbuff);
 strcat(achAddr,gstConfigs.achServerIP);
 printf("Server IP = %s\n", achAddr);

}

int setAddrIpv6(struct sockaddr_in6 *addr_ipv6,uint16_t port,char *ipv6){
        if(addr_ipv6&&ipv6){
                addr_ipv6->sin6_family=AF_INET6;
                addr_ipv6->sin6_port=htons(port);
                if(inet_pton(AF_INET6,ipv6,&(addr_ipv6->sin6_addr))<0){
                        perror("Ipv6 address assignment failed");
                }
                return 0;
        }
        return -1;
}


void RequestServID()
{

   struct sockaddr_in6 addr_ipv6 = {0};
   int8 achAddr[MAX_LINE_LENGTH] = {0}; 
   int8 achRqstMsg[MAX_LINE_LENGTH] = {0}; 
   int32 i32Retval = 0;
   int size = sizeof(struct sockaddr_in6);
   int8 * pi8SavePtr = NULL;
   int8 * pi8Token = NULL;
   int8 * pi8Value = NULL;
   
   if(strlen(gstConfigs.achServerIP) <= IPV4_ADDR_LEN)
   {
       MapV4toV6(achAddr);
   }
   else
   {
       memcpy(achAddr , gstConfigs.achServerIP , strlen(gstConfigs.achServerIP));
   }
   i32Retval =  setAddrIpv6(&addr_ipv6,gstConfigs.ui32Port,achAddr);

   if(0 == i32Retval)
   {
        /*TODO: create another socket */
        if(sendto(gUDPServSockFD,achRqstMsg,strlen(achRqstMsg),0,(struct sockaddr *)&addr_ipv6,size) > 0)
        {
                if(recvfrom(gUDPServSockFD ,achRqstMsg ,strlen(achRqstMsg) , 0 , NULL, NULL) > 0)
                {
                     pi8Token = strtok_r(achRqstMsg ,DELIMITER , &pi8SavePtr);  
                     pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);
                      if(0 == strcmp(pi8Token, MSERVERID) && (0 == strcmp(pi8Value, "SERVERID")))
                      {
                         pi8Token = strtok_r(NULL ,DELIMITER , &pi8SavePtr);  
                         gstConfigs.ui64ServID = strtol(pi8Value,NULL,0);
                         /*Write to config file*/
                         ChangeServID();
                      }
                } 
        }

   }
}


stRcvdMsg * CreateMsg()
{
 stRcvdMsg * pstNewMsg = NULL;

 pstNewMsg = malloc(sizeof(stRcvdMsg));
 
 if(NULL != pstNewMsg)
 {
    if(NULL != (pstNewMsg->strcvd_addr = malloc(sizeof(struct sockaddr_in6))))
    {
      memset(pstNewMsg->strcvd_addr , 0, sizeof(struct sockaddr_in6));
      if(NULL == (pstNewMsg->achBuffer = malloc(MAX_MSG_LEN)))
      {
        free(pstNewMsg->strcvd_addr);
        free(pstNewMsg);
        return NULL;
      }
      else
      {
          memset(pstNewMsg->achBuffer , 0, MAX_MSG_LEN);
      } 
    }
    else
    {
       free(pstNewMsg);
       return NULL;
    }
 } 
 return pstNewMsg;
}



void * ProcessThreadStart()
{

    int8 datagram[MAX_MSG_LEN] = {0};
    struct sockaddr_in6 cli_addr6 = {0};
    stRcvdMsg * pstNewMsg = NULL;
    socklen_t len = 0;
    int32 n = 0;
    uint32 priority = 1;
    int32 isset = 0;
     int32 maxfd = 0;
     fd_set readset = {{0}};
     fd_set copyset = {{0}};

     FD_ZERO(&readset);
     FD_SET(gUDPCliSockFD , &readset);
     FD_SET(gUDPServSockFD , &readset);
     copyset = readset;
     maxfd = (gUDPCliSockFD > gUDPServSockFD ? gUDPCliSockFD : gUDPServSockFD)+ 1;
     len = sizeof(cli_addr6);

     while(1)
     {

     readset = copyset;
     if(select(maxfd , &readset , NULL ,NULL , NULL)  >= 0)
     {

         memset(datagram , 0 , MAX_MSG_LEN);
         if(NULL == (pstNewMsg = CreateMsg()))
         {continue; }
     
        if(FD_ISSET(gUDPCliSockFD , &readset))
        {
                if ((n = recvfrom(gUDPCliSockFD, datagram, MAX_MSG_LEN, 0,
                          (struct sockaddr *) pstNewMsg->strcvd_addr, &len)) < 0) {
                        printf("ERROR: recvfrom : number of bytes received %d\n",n);
                        perror("recvfrom");
                        return NULL;
                }
                isset = 1;
        }

        if(FD_ISSET(gUDPServSockFD , &readset))
        {
                if ((n = recvfrom(gUDPServSockFD, datagram, MAX_MSG_LEN, 0,
                          (struct sockaddr *) pstNewMsg->strcvd_addr, &len)) < 0) {
                        printf("ERROR: recvfrom : number of bytes received %d\n",n);
                        perror("recvfrom");
                        return NULL;
                }
                isset = 1;
        }
       
        if(isset)
        {
                isset = 0;
                memcpy(pstNewMsg->achBuffer , datagram,n);
                if((mq_send(gMsgQID, (char *) pstNewMsg, sizeof(stRcvdMsg), priority)) < 0 )
                {
                   printf("ERROR: mq_send : packet can not be put in the mq\n");
                }
         }
       }
    } 
return NULL;
}

