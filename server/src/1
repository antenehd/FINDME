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

uint64 gi64ServID = 0;


stRecord *  CreateRecord(uint64 ui64ID)
{

/*Create a node and add to the link list*/

return NULL;
}



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
  uint64 ui64ID = 0; 
  /*read from the config file for the range applicable to client*/

 /*Then for the message and send to client*/
 
 printf("In fucntion %s\n",__FUNCTION__);

  CreateRecord(ui64ID);
  freeMsg(pstRcvdMsg);
}


stRecord * SearchRecord(uint64 ui64Id)
{
  /*Start from the head pointer and search linearly*/
 printf("In fucntion %s\n",__FUNCTION__);
 
 return NULL;
}

void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,stRecord * pstRedAdd,uint32 ui32BitMask)
{
 
  int8 * pachRepBuff = NULL;

  pachRepBuff = (int8 *) malloc(MAX_MSG_LEN); 
  printf("In fucntion %s\n",__FUNCTION__);
 
  if((NULL != pstRcvdMsg) && (NULL != pachRepBuff))
  {
      memset(pachRepBuff , 0 , MAX_MSG_LEN);
      snprintf(pachRepBuff, sizeof(uint64),"%8ld",gi64ServID);
      strcat(pachRepBuff , DELIMITER);
      if((ui32BitMask & CLIENT) != 0)
      {
          strcat(pachRepBuff , REPLY);
      }
      else
      {
          if((ui32BitMask & UPDATE) != 0)
          {
              strcat(pachRepBuff , "UPDATE");  
          }
          else if((ui32BitMask & NEW) != 0)
          {
              strcat(pachRepBuff , "NEW");      
          }    
      }
      strcat(pachRepBuff , DELIMITER);

      if((ui32BitMask & NAME) != 0)
      {
         strcat(pachRepBuff , "NAME");
         strcat(pachRepBuff , DELIMITER);
         strcat(pachRepBuff,pstRedAdd->achName);          
         strcat(pachRepBuff , DELIMITER);
      }
      else if((ui32BitMask & EMAIL) != 0)
      {
         strcat(pachRepBuff , "EMAIL");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achEmail);          
         strcat(pachRepBuff , DELIMITER);

      }
      else if((ui32BitMask & ADDRESS) != 0)
      {
         strcat(pachRepBuff , "ADDRESS");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achAddr);          
         strcat(pachRepBuff , DELIMITER);

      }
      else if((ui32BitMask & LOCATION) != 0)
      {
         strcat(pachRepBuff , "LOCATION");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achLastKnownLoc);          
         strcat(pachRepBuff , DELIMITER);

      }
      else if((ui32BitMask & NOTYPE) != 0)
      {
         strcat(pachRepBuff , "NOTYPE");
         strcat(pachRepBuff , DELIMITER);          

      }
      else if((ui32BitMask & NOREC) != 0)
      {
         strcat(pachRepBuff , "NOREC");
         strcat(pachRepBuff , DELIMITER);          

      }
      pachRepBuff[strlen(pachRepBuff)] = '\0';
     /*TODO: Add sending part*/ 
     freeMsg(pstRcvdMsg);
  }
 
}

void HandleCliQuery(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr,uint64 ui64ID)
{
  /*Start decoding after the message type field*/

 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 int8 * pi8Value = NULL;
 uint64 ui64RecId = 0;
 uint32 ui32BitMask = 0;
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
              ui32BitMask |= EMAIL|ADDRESS|LOCATION;
         }
         else if(0 == strcmp(pi8Token,"EMAIL"))
         {
              ui32BitMask |= NAME|ADDRESS|LOCATION;
         }
         else if(0 == strcmp(pi8Token,"LOCATION"))
         {
              ui32BitMask |= NAME|EMAIL|ADDRESS;
         }
         else if(0 == strcmp(pi8Token,"ADDRESS"))
         {
              ui32BitMask |= NAME|EMAIL|LOCATION;
         }
         else
         {
              ui32BitMask |= NOTYPE;
             /*Send error message for unknown query*/
         }
     } 
    else
    {
       /*Requested Client not found , send error message to client*/
              ui32BitMask |= NOREC;
       
    }
    PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);
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
   stRecord * pstRedAdd = NULL;
   uint32 ui32BitMask = 0;

    if(NULL != (pstRedAdd = SearchRecord(ui64ID)))
    {     
        if((ui32BitMask = FillRecord(pstRedAdd,pi8MsgPtr)) != 0)
        {
           if(pstRedAdd->ui32ContentFull == 0)
            {
              ui32BitMask |= SERVER|NEW;
            }
            else
            {
              ui32BitMask |= SERVER|UPDATE;
            }
        }     
        PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);   
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


int isServer(ui64ID)
{


return 0;
}

uint32 FillRecord(stRecord * pstActRec,int8 * pi8MsgPtr)
{
 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 int8 * pi8Value = NULL;
 stRecord * pstRedAdd = NULL;
 uint32 ui32BitMask = 0;
 pstRedAdd = malloc(sizeof(stRecord));

 if((NULL != pstRedAdd) && (NULL != pi8MsgPtr) && (NULL != pstActRec))
 {
    memset(pstActRec,0,sizeof(sizeof(stRecord)));
    pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
    pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);
 
  while(NULL != pi8Token)
  {

	  if(0 == strcmp(pi8Token,"NAME"))
	  {
		  strncpy(pstRedAdd->achName ,pi8Value, strlen(pi8Value));
                  ui32BitMask |= NAME;
	  }
	  else if(0 == strcmp(pi8Token,"EMAIL"))
	  {
		  strncpy(pstRedAdd->achEmail ,pi8Value, strlen(pi8Value));
                  ui32BitMask |= EMAIL;
	  }
	  else if(0 == strcmp(pi8Token,"LOCATION"))
	  {
		  strncpy(pstRedAdd->achLastKnownLoc ,pi8Value, strlen(pi8Value));
                  ui32BitMask |= LOCATION;
	  }
	  else if(0 == strcmp(pi8Token,"ADDRESS"))
	  {
		  strncpy(pstRedAdd->achAddr ,pi8Value, strlen(pi8Value));
                  ui32BitMask |= ADDRESS;
	  }
          else if(0 == strcmp(pi8Token,"TIMESTAMP"))
	  {
                  pstRedAdd->ui64LastUpdate = strtol(pi8Value,NULL,0);
                  ui32BitMask |= TIMESTAMP;
	  }
          pi8Token = strtok_r(NULL , DELIMITER , &pi8SavePtr);
	  pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);
   } 

   /*update only if it is latest*/
   if(pstActRec->ui64LastUpdate <  pstRedAdd->ui64LastUpdate)
   {
      if((NAME & ui32BitMask)!= 0)
      {
         strncpy(pstActRec->achName, pstRedAdd->achName ,strlen(pstRedAdd->achName));
      }
      else if((EMAIL & ui32BitMask)!= 0)
      {
         strncpy(pstActRec->achEmail, pstRedAdd->achEmail,strlen(pstRedAdd->achEmail));
      }
      if((ADDRESS & ui32BitMask)!= 0)
      {
         strncpy(pstActRec->achAddr, pstRedAdd->achAddr ,strlen(pstRedAdd->achAddr));
      }
      if((LOCATION & ui32BitMask)!= 0)
      {
         strncpy(pstActRec->achLastKnownLoc, pstRedAdd->achLastKnownLoc ,strlen(pstRedAdd->achLastKnownLoc));
      }	
      else if((TIMESTAMP & ui32BitMask)!= 0)
      {
         pstActRec->ui64LastUpdate = pstRedAdd->ui64LastUpdate;
      }
   }
   free(pstRedAdd);
 } 

 return ui32BitMask;
}


void  HandleServNew(stRcvdMsg * pstRcvdMsg ,int8 * pi8MsgPtr,uint64 ui64ID)
{
 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 uint64 ui64RecId = 0;
 stRecord * pstRedAdd = NULL;

 if(NULL != pi8MsgPtr)
 { 
    pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
    ui64RecId = strtol(pi8Token,NULL,0);
    if(NULL == (pstRedAdd = SearchRecord(ui64RecId)))
    {
       pstRedAdd = CreateRecord(ui64RecId);
       if(NULL != pstRedAdd)
       {
            pstRedAdd->ui64RecNum = ui64RecId;
            FillRecord(pstRedAdd,pi8MsgPtr);
       }	           
    }
    else
    {
       /*Client already exists,just update*/      
        FillRecord(pstRedAdd,pi8MsgPtr); 
    }
  freeMsg(pstRcvdMsg);
 }
}

void  HandleServUpdate(stRcvdMsg * pstRcvdMsg ,int8 *   pi8MsgPtr)
{

stRecord * pstRedAdd = NULL;
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;
uint64 ui64RecId = 0;

if(NULL !=  pstRcvdMsg && (NULL != pi8MsgPtr))
{
    
  pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
  ui64RecId = strtol(pi8Token,NULL,0);

  /*Look for the client record and then update the client*/
    if(NULL != (pstRedAdd = SearchRecord(ui64RecId)))
    {     
        FillRecord(pstRedAdd,pi8MsgPtr);
    }
  freeMsg(pstRcvdMsg);
}
}

void HandleServerReceivedMsg(stRcvdMsg * pstRcvdMsg)
{

int8*  pachTestArray = pstRcvdMsg->achBuffer;
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;
uint64 ui64ID = 0;

if(NULL !=  pstRcvdMsg)
{
  /*Check for the ID*/
        pi8Token = strtok_r(pachTestArray ,DELIMITER , &pi8SavePtr);
        ui64ID = strtol(pi8Token,NULL,0);

        if(0 == isServer(ui64ID))
	{
            /*validate client and hendle the client*/
            pi8Token = strtok_r (NULL, DELIMITER , &pi8SavePtr);
            if(0 == (strcmp(pi8Token , "NEW")))
            {
               HandleServNew(pstRcvdMsg , pi8SavePtr,ui64ID);
            }
            if(0 == (strcmp(pi8Token , "UPDATE")))
            {
               HandleServUpdate(pstRcvdMsg , pi8SavePtr);
            }
        }
        else
        {
           printf("Unknown Server\n");
        }
 	
}
}
