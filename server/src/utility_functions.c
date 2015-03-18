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

uint64 gi64ServID = 10000000;
uint64 gi64StartID;
uint64 gi64EndID = 1000000;

extern HashTable_t  *gpHashTable;


HashTable_t * CreateHash(int32 i32size)
{

HashTable_t * HashTable = NULL;
int32 i32Count = 0;
if(i32size > 65535 || i32size < 1)
return NULL;

HashTable = malloc(sizeof(HashTable_t));
HashTable->i32size = i32size;
HashTable->pstTable = malloc(sizeof(HashRecord_t *) * i32size);

if(NULL != HashTable)
{
    for(i32Count = 0; i32Count < i32size ; i32Count++)
    { 
        HashTable->pstTable[i32Count] = malloc(sizeof(HashRecord_t));
        HashTable->pstTable[i32Count]->i32key = i32Count;
        HashTable->pstTable[i32Count]->pstValue = NULL;
        HashTable->pstTable[i32Count]->pstnext = NULL;
    }
return HashTable;
} 
return NULL;
}


void InsertRecord(stRecord * pstNewRecord)
{
  uint32 ui32Key;
  HashRecord_t * pstNewHashRec = NULL;
  HashRecord_t * pstLastHashRec = NULL;

  ui32Key = (pstNewRecord->ui64RecNum % gpHashTable->i32size);

 /*TODO:acquire mutex here*/
  if(NULL != pstNewRecord)
  {
     if(gpHashTable->pstTable[ui32Key]->pstValue == NULL) 
     {
       gpHashTable->pstTable[ui32Key]->pstValue = pstNewRecord;
     }
     else
     {
       pstLastHashRec = gpHashTable->pstTable[ui32Key];

       while(pstLastHashRec->pstnext != NULL)
       {
          pstLastHashRec = pstLastHashRec->pstnext;
       }

       pstNewHashRec = malloc(sizeof(HashRecord_t));
       if(NULL != pstNewHashRec)
       {
           pstNewHashRec->i32key = ui32Key;
           pstNewHashRec->pstValue = pstNewRecord;
           pstNewHashRec->pstnext=  NULL;
           pstLastHashRec->pstnext = pstNewHashRec;
       }
     }
  }

}

stRecord *  CreateRecord(uint64 ui64ID)
{
   stRecord * pstNewRecord = NULL;

   pstNewRecord = malloc(sizeof(stRecord));
   if(NULL != pstNewRecord)
   {
       /*Create a node and add to the link list*/
       pstNewRecord->ui64RecNum = ui64ID;
       InsertRecord(pstNewRecord);
   }
   return pstNewRecord;
}

/*TODO :
1) create message
2) create record
3) Assign ID
4) Read configurations
5) Create message queues
6) Implement search*/

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

  static uint64 ui64ClientID = 1;
  uint32 ui32BitMask = 0;
  stRecord * pstRedAdd = NULL;

  /*read from the config file for the range applicable to client*/

 /*Then for the message and send to client*/
  if((ui64ClientID > gi64StartID)  && (ui64ClientID < gi64EndID))
  {
     pstRedAdd = CreateRecord(gi64ServID+ui64ClientID);
     ui32BitMask += CLIENT|CLIENTID;
     PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);
     ui64ClientID++;
     /*TODO : Prepare and send*/
  }

 
 printf("In fucntion %s\n",__FUNCTION__);

}


stRecord * SearchRecord(uint64 ui64Id)
{

  int32 i32key = 0;
  HashRecord_t * pstPlaceHolder = NULL;

  /*Start from the head pointer and search linearly*/
 printf("In fucntion %s\n",__FUNCTION__);
 /*TODO: acquire mutex here*/
 i32key = ui64Id % gpHashTable->i32size;
 pstPlaceHolder = gpHashTable->pstTable[i32key];
 if((pstPlaceHolder->pstValue != NULL))
 {
    if((pstPlaceHolder->pstnext != NULL)) 
   {
       while(( pstPlaceHolder->pstnext != NULL  ) && 
            (pstPlaceHolder->pstValue->ui64RecNum != ui64Id))          
       {
           pstPlaceHolder = pstPlaceHolder->pstnext;
       }       
   }
   if((NULL != pstPlaceHolder->pstValue) && 
     (pstPlaceHolder->pstValue->ui64RecNum == ui64Id))
     {  
        return pstPlaceHolder->pstValue;
     }
 }  
   return NULL; 
}

void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,stRecord * pstRedAdd,uint32 ui32BitMask)
{
 
  int8 * pachRepBuff = NULL;
  int8 achBuff[20] = {0};

  pachRepBuff = (int8 *) malloc(MAX_MSG_LEN); 
  printf("In fucntion %s\n",__FUNCTION__);
 
  if((NULL != pstRcvdMsg) && (NULL != pachRepBuff))
  {
      memset(pachRepBuff , 0 , MAX_MSG_LEN);
      snprintf(pachRepBuff, sizeof(uint64),"%ld",gi64ServID);
      strcat(pachRepBuff , DELIMITER);
      if((ui32BitMask & CLIENT) != 0)
      {
          strcat(pachRepBuff , REPLY);
      }
      else if((ui32BitMask & SERVER) != 0)
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
      else
      {
        /*unknown type*/
          free(pachRepBuff);
          return;
      }
      strcat(pachRepBuff , DELIMITER);
      
      if((ui32BitMask & CLIENTID) != 0)
      {
         strcat(pachRepBuff , "CLIENTID");
         strcat(pachRepBuff , DELIMITER);
         snprintf(achBuff , 20,"%ld", pstRedAdd->ui64RecNum);
         strcat(pachRepBuff,achBuff);          
         strcat(pachRepBuff , DELIMITER);
      }
      if((ui32BitMask & NAME) != 0)
      {
         strcat(pachRepBuff , "NAME");
         strcat(pachRepBuff , DELIMITER);
         strcat(pachRepBuff,pstRedAdd->achName);          
         strcat(pachRepBuff , DELIMITER);
      }      
      if((ui32BitMask & EMAIL) != 0)
      {
         strcat(pachRepBuff , "EMAIL");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achEmail);          
         strcat(pachRepBuff , DELIMITER);

      }
      if((ui32BitMask & ADDRESS) != 0)
      {
         strcat(pachRepBuff , "ADDRESS");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achAddr);          
         strcat(pachRepBuff , DELIMITER);

      }
      if((ui32BitMask & LOCATION) != 0)
      {
         strcat(pachRepBuff , "LOCATION");
         strcat(pachRepBuff , DELIMITER);          
         strcat(pachRepBuff,pstRedAdd->achLastKnownLoc);          
         strcat(pachRepBuff , DELIMITER);

      }
      if((ui32BitMask & NOTYPE) != 0)
      {
         strcat(pachRepBuff , "NOTYPE");
         strcat(pachRepBuff , DELIMITER);          

      }
      if((ui32BitMask & NOREC) != 0)
      {
         strcat(pachRepBuff , "NOREC");
         strcat(pachRepBuff , DELIMITER);          

      }
      pachRepBuff[strlen(pachRepBuff)-1] = '\0';
      printf("Buffer: %s\n",pachRepBuff);
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
              ui32BitMask |= EMAIL|ADDRESS|LOCATION|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"EMAIL"))
         {
              ui32BitMask |= NAME|ADDRESS|LOCATION|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"LOCATION"))
         {
              ui32BitMask |= NAME|EMAIL|ADDRESS|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"ADDRESS"))
         {
              ui32BitMask |= NAME|EMAIL|LOCATION|CLIENT;
         }
         else
         {
              ui32BitMask |= NOTYPE|CLIENT;
             /*Send error message for unknown query*/
         }
     } 
    else
    {
       /*Requested Client not found , send error message to client*/
              ui32BitMask |= NOREC|CLIENT;
       
    }
    PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);
 }
     
}


int32 isclient(uint64 ui64ID)
{
uint64 ui64Temp =ui64ID - gi64ServID; 
/*Use the range specified in the configfile, this is just a test value*/
   if((ui64Temp > gi64StartID)  && (ui64Temp < gi64EndID))
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
              ui32BitMask |= SERVER|UPDATE|CLIENTID;
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
            else if(0 == (strcmp(pi8Token , "UPDATE")))
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
    memset(pstRedAdd,0,sizeof(sizeof(stRecord)));
    pstRedAdd->ui64LastUpdate = 0;
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
   if((pstActRec->ui64LastUpdate <  pstRedAdd->ui64LastUpdate) || (pstActRec->ui32ContentFull == 0))
   {
      pstActRec->ui32ContentFull = 1; 
      if((NAME & ui32BitMask)!= 0)
      {
         strncpy(pstActRec->achName, pstRedAdd->achName ,strlen(pstRedAdd->achName));
      }
      if((EMAIL & ui32BitMask)!= 0)
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
      if((TIMESTAMP & ui32BitMask)!= 0)
      {
         pstActRec->ui64LastUpdate = pstRedAdd->ui64LastUpdate;
      }
   }
   else
   {
      ui32BitMask = 0;
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
  if(0 == strcmp(pi8Token , "CLIENTID"))
  {
     pi8Token = strtok_r(NULL , DELIMITER , &pi8SavePtr);
     ui64RecId = strtol(pi8Token,NULL,0);

  /*Look for the client record and then update the client*/
     if(NULL != (pstRedAdd = SearchRecord(ui64RecId)))
     {     
        FillRecord(pstRedAdd,pi8SavePtr);
     }
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
            else if(0 == (strcmp(pi8Token , "UPDATE")))
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
