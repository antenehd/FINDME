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

void freeMsg(stRcvdMsg * pstRcvdMsg)
{

  if(NULL != pstRcvdMsg)
  {
    if(NULL != pstRcvdMsg->strcvd_addr)
    {free( pstRcvdMsg->strcvd_addr);}
    
    if(NULL != pstRcvdMsg->achBuffer)
    {free( pstRcvdMsg->achBuffer);}
   
    free(pstRcvdMsg);
  }
}


void AssignIDToCli(stRcvdMsg * pstRcvdMsg)
{
  /*read from the config file for the range applicable to client*/

 /*Then for the message and send to client*/
 
 printf("In fucntion %s\n",__FUNCTION__);

  freeMsg(pstRcvdMsg);
}


stRecord * SearchRecord(uint64 ui64Id)
{
  /*Start from the head pointer and search linearly*/
 printf("In fucntion %s\n",__FUNCTION__);
 
 return NULL;
}

void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,int32 MsgCode)
{

  printf("In fucntion %s\n",__FUNCTION__);
 
  freeMsg(pstRcvdMsg);
}

void HandleCliQuery(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr,uint64 ui64ID)
{
  /*Start decoding after the message type field*/

 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 int8 * pi8Value = NULL;
 uint64 ui64RecId = 0;
 stRecord * pstRedAdd = NULL;

 if(NULL != pi8MsgPtr)
 { 
    pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
    ui64RecId = strtol(pi8Token,NULL,0);
    if(NULL != (pstRedAdd = SearchRecord(ui64RecId)))
    {
         pi8Token = strtok_r(NULL , DELIMITER , &pi8SavePtr);
         pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);
         if(0 == strcmp(pi8Token,"NAME"))
         {
              PrepareCliRsp(pstRcvdMsg,NAME);
         }
         else if(0 == strcmp(pi8Token,"EMAIL"))
         {
              PrepareCliRsp(pstRcvdMsg,EMAIL);
         }
         else if(0 == strcmp(pi8Token,"LOCATION"))
         {
              PrepareCliRsp(pstRcvdMsg,LOCT);
         }
         else if(0 == strcmp(pi8Token,"ADDRESS"))
         {
              PrepareCliRsp(pstRcvdMsg,ADDR);
         }
         else
         {
              PrepareCliRsp(pstRcvdMsg,NOTYPE);
             /*Send error message for unknown query*/
         }
    }
    else
    {
       /*Requested Client not found , send error message to client*/
              PrepareCliRsp(pstRcvdMsg,NOREC);
       
    }
 }
     
}


int32 isclient(uint64 ui64ID)
{
 
/*Use the range specified in the configfile, this is just a test value*/
   if((ui64ID > 10000000) && (ui64ID < 20000000))
   {
      return 0;
   }
return -1;
}

void HandleCliUpdate(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr ,uint64 ui64ID)
{
  /*Start decoding after the message type field*/
  /*TODO: update the location*/
 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 uint64 ui64RecId = 0;
 stRecord * pstRedAdd = NULL;
 int8 * pi8Value = NULL;

    if(NULL != (pstRedAdd = SearchRecord(ui64ID)) && (0 !=  pstRedAdd->i32NoRec ))
    {     
        while(NULL != pi8Token)
        {
           pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
           if(NULL != pi8Token)
	   {
		   pi8Value = strtok_r(NULL, DELIMITER , &pi8SavePtr);

		   if(0 == strcmp(pi8Token,"NAME"))
		   {
			   strncpy(pstRedAdd->achName ,pi8Value, strlen(pi8Value));
		   }
		   else if(0 == strcmp(pi8Token,"EMAIL"))
		   {
			   strncpy(pstRedAdd->achEmail ,pi8Value, strlen(pi8Value));
		   }
		   else if(0 == strcmp(pi8Token,"LOCATION"))
		   {
			   strncpy(pstRedAdd->achLastKnownLoc ,pi8Value, strlen(pi8Value));
		   }
		   else if(0 == strcmp(pi8Token,"ADDRESS"))
		   {
			   strncpy(pstRedAdd->achAddr ,pi8Value, strlen(pi8Value));
		   }
		   else
		   {
			   PrepareCliRsp(pstRcvdMsg,NOTYPE);
			   /*Send error message for unknown query*/
		   }  
	   }
       }
    }
}


int32  HandleClientReceivedMsg(stRcvdMsg * pstRcvdMsg)
{

int8*  pachTestArray = pstRcvdMsg->achBuffer; 
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;
uint64 ui64ID = 0;

if(NULL != pstRcvdMsg)
{
        /*Check for the ID*/
        pi8Token = strtok_r(pachTestArray ,DELIMITER , &pi8SavePtr);
        ui64ID = strtol(pi8Token,NULL,0);

        if(0 == ui64ID)
	{
           AssignIDToCli(pstRcvdMsg);
	}
        else if(0 == isclient(ui64ID))
        {
            /*validate client and hendle the client*/
            pi8Token = strtok_r (NULL, DELIMITER , &pi8SavePtr);
            if(0 == (strcmp(pi8Token , "QUERY")))
            {
               HandleCliQuery(pstRcvdMsg , pi8SavePtr,ui64ID);
            }
            if(0 == (strcmp(pi8Token , "UPDATE")))
            {
               HandleCliUpdate(pstRcvdMsg , pi8SavePtr,ui64ID);
            } 
        }
        else
        {
           printf("Unknown client\n");
        }

}	
return 0;
}
