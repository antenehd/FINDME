#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>
#include <mqueue.h>

#include "data_structure.h"
#include "proto_types.h"

uint64 gi64StartID = 1;
uint64 gi64EndID = 100;

extern int32 gUDPCliSockFD;
extern int32 gUDPServSockFD;
extern HashTable_t  *gpHashTable;
extern pthread_mutex_t stRecMutex;
FILE * fpLog;
extern stConfigFileItems gstConfigs;

void PrintAllRecord()
{

  int32 i32Count = 0;
  HashRecord_t *  pstNextRec = NULL;
  /*Print all the record in the hash table before exiting*/
  if(0 == pthread_mutex_lock(&stRecMutex))
  {
    if(NULL != gpHashTable)
   {
    if(gpHashTable->pstTable != NULL)
    {
        for(i32Count = 0; i32Count < gpHashTable->i32size ; i32Count++) 
        {
              if(gpHashTable->pstTable[i32Count] != NULL)
              {
                  if(gpHashTable->pstTable[i32Count]->pstValue != NULL)
                  {
                      printf("ID : %ld\n",gpHashTable->pstTable[i32Count]->pstValue->ui64RecNum); 
                      printf("Name : %s\n",gpHashTable->pstTable[i32Count]->pstValue->achName); 
                      printf("Email : %s\n",gpHashTable->pstTable[i32Count]->pstValue->achEmail); 
                      printf("Address : %s\n",gpHashTable->pstTable[i32Count]->pstValue->achAddr); 
                      printf("Location : %s\n\n",gpHashTable->pstTable[i32Count]->pstValue->achLastKnownLoc); 
                  }

                  pstNextRec = gpHashTable->pstTable[i32Count]->pstnext;
                  while( pstNextRec != NULL)
                  {
                         
                      if(pstNextRec->pstValue != NULL)
                      {
                          printf("ID : %ld\n",pstNextRec->pstValue->ui64RecNum); 
                          printf("Name : %s\n",pstNextRec->pstValue->achName); 
                          printf("Email : %s\n",pstNextRec->pstValue->achEmail); 
                          printf("Address : %s\n",pstNextRec->pstValue->achAddr); 
                          printf("Location : %s\n\n",pstNextRec->pstValue->achLastKnownLoc); 
                      }
                  }
              }
        }       
    } 
  }
 }
  pthread_mutex_unlock(&stRecMutex);
}


void sighandler_SIGINT()
{

  int32 i32Count = 0;
  HashRecord_t *  pstNextRec = NULL;
  printf("IN SIGNAL HANDLER\n");
  PrintAllRecord();
  /*Before exiting the process free all  allocated the memory*/
  if(0 == pthread_mutex_lock(&stRecMutex))
  {
    if(NULL != gpHashTable)
   {
    if(gpHashTable->pstTable != NULL)
    {
        for(i32Count = 0; i32Count < gpHashTable->i32size ; i32Count++) 
        {
              if(gpHashTable->pstTable[i32Count] != NULL)
              {
                  if(gpHashTable->pstTable[i32Count]->pstValue != NULL)
                  {free(gpHashTable->pstTable[i32Count]->pstValue); }

                  while( gpHashTable->pstTable[i32Count]->pstnext != NULL)
                  {
                      pstNextRec = gpHashTable->pstTable[i32Count]->pstnext->pstnext;
                         
                      if(gpHashTable->pstTable[i32Count]->pstnext->pstValue != NULL)
                      {
                         free(gpHashTable->pstTable[i32Count]->pstnext->pstValue); 
                         free(gpHashTable->pstTable[i32Count]->pstnext); 
                         gpHashTable->pstTable[i32Count]->pstnext = pstNextRec; 
                      }
                  }
                  free(gpHashTable->pstTable[i32Count]); 
              }
        }       
        free(gpHashTable->pstTable); 
        free(gpHashTable); 
        gpHashTable = NULL; 
    } 
  }
 }
  pthread_mutex_unlock(&stRecMutex);
  mq_unlink(MQ_NAME);
  fflush(fpLog);
	disjoinMstSrv();
  exit(0);
}


HashTable_t * CreateHash(int32 i32size)
{

/* create and Initialize each of the members inside the hash 
table */
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

 /*Insert a new record baased on the client ID
  use client Id as the key to insert the record
*/
 
  ui32Key = (pstNewRecord->ui64RecNum % gpHashTable->i32size);

  if(0 == pthread_mutex_lock(&stRecMutex))
  {
      if(NULL != pstNewRecord)
      {
          /*If is the only recored in the bucket*/
          if(gpHashTable->pstTable[ui32Key]->pstValue == NULL) 
          {
             gpHashTable->pstTable[ui32Key]->pstValue = pstNewRecord;
          }
          else
          {/*Traverse till the last and insert this record*/
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
    pthread_mutex_unlock(&stRecMutex);
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
       pstNewRecord->ui32ContentFull = 0;
   }
   return pstNewRecord;
}


void freeMsg(stRcvdMsg * pstRcvdMsg)
{
  /*Free all the allocations for the message*/
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

  /*Check for the range applicable to client, and assign an ID*/

  if((ui64ClientID >= gi64StartID)  && (ui64ClientID < gi64EndID))
  {
     pstRedAdd = CreateRecord(gstConfigs.ui64ServID+ui64ClientID);
     ui32BitMask += CLIENT|CLIENTID;
     PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);
     ui64ClientID++;
  }
  
}


stRecord * SearchRecord(uint64 ui64Id)
{

  int32 i32key = 0;
  HashRecord_t * pstPlaceHolder = NULL;
  stRecord * pstRetval = NULL;

  if(0 == pthread_mutex_lock(&stRecMutex))
  {
     /*Client Id as the key*/
     i32key = ui64Id % gpHashTable->i32size;
     pstPlaceHolder = gpHashTable->pstTable[i32key];
     if((pstPlaceHolder->pstValue != NULL))
    {
        if((pstPlaceHolder->pstnext != NULL)) 
        {
             /*If there are more than one record in the bucket*/
             while(( pstPlaceHolder->pstnext != NULL  ) && 
              (pstPlaceHolder->pstValue->ui64RecNum != ui64Id))          
             {
                 pstPlaceHolder = pstPlaceHolder->pstnext;
             }       
        }
        if((NULL != pstPlaceHolder->pstValue) && 
        (pstPlaceHolder->pstValue->ui64RecNum == ui64Id))
        {  /*If record exist for the ID*/
            pstRetval = pstPlaceHolder->pstValue;
        }
    }
    pthread_mutex_unlock(&stRecMutex);
  }  
    return pstRetval; 
}


stRecord * SearchRecord_New(uint32 ui32QueryMask , int8 *pi8Query)
{
  uint64 ui64RecId = 0;
  int32 i32key = 0;
  int32 i32Found = 0;
  HashRecord_t * pstPlaceHolder = NULL;
  stRecord * pstRetval = NULL;

 if(0 == pthread_mutex_lock(&stRecMutex))
  {
     i32key = 0;
     /*Client Id as the key*/
     while(i32key <= gpHashTable->i32size)
     {
         pstPlaceHolder = gpHashTable->pstTable[i32key];
     
        while((pstPlaceHolder->pstValue != NULL))
        {

              switch(ui32QueryMask)
              {
                case NAME :

                         if(0 == strcmp(pi8Query , pstPlaceHolder->pstValue->achName))
                         {
                              pstRetval = pstPlaceHolder->pstValue;
                              i32Found = 1;
                         }
                              break;
                case EMAIL : 
                         if(0 == strcmp(pi8Query , pstPlaceHolder->pstValue->achEmail))
                         {
                              pstRetval = pstPlaceHolder->pstValue;
                              i32Found = 1;
                         }
                             break;
                case ADDRESS : 
                         if(0 == strcmp(pi8Query , pstPlaceHolder->pstValue->achAddr))
                         {
                              pstRetval = pstPlaceHolder->pstValue;
                              i32Found = 1;
                         }
                             break;
                case LOCATION : 
                         if(0 == strcmp(pi8Query , pstPlaceHolder->pstValue->achLastKnownLoc))
                         {
                              pstRetval = pstPlaceHolder->pstValue;
                              i32Found = 1;
                         }
                             break;
                case CLIENTID :
                                   ui64RecId = strtol(pi8Query,NULL,10);
                         if( ui64RecId == pstPlaceHolder->pstValue->ui64RecNum)
                         {
                              pstRetval = pstPlaceHolder->pstValue;
                              i32Found = 1;
                         }
                         break;
                default:
                        break;
               }
 
           if((pstPlaceHolder->pstnext != NULL)) 
           {
             /*If there are more than one record in the bucket*/
                 pstPlaceHolder = pstPlaceHolder->pstnext;
           }
           if(i32Found == 1)
           {
              pthread_mutex_unlock(&stRecMutex);
              return pstRetval;
           }
        }
        
        i32key++;

    }
    pthread_mutex_unlock(&stRecMutex);
  }  
    return pstRetval; 
}


void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,stRecord * pstRedAdd,uint32 ui32BitMask)
{
 
  int8 * pachRepBuff = NULL;
  int8 achBuff[20] = {0};
  int32 bytes = 0;
  int32 size = sizeof(struct sockaddr_in6);

  pachRepBuff = (int8 *) malloc(MAX_MSG_LEN); 
  /*For all the bits set in ui32BitMask , encode the 
  parameter inside the mesage*/
  if((NULL != pstRcvdMsg) && (NULL != pachRepBuff))
  {
      memset(pachRepBuff , 0 , MAX_MSG_LEN);
      if((ui32BitMask & CLIENT) != 0)
      {
          snprintf(pachRepBuff, sizeof(uint64),"%ld",
                    gstConfigs.ui64ServID);
          strcat(pachRepBuff , DELIMITER);
          /*Message to another client*/
          strcat(pachRepBuff , REPLY);
      }
      else if((ui32BitMask & SERVER) != 0)
      {
          /*Message to another Server*/
          if((ui32BitMask & UPDATE) != 0)
          {
              strcpy(pachRepBuff , "UPDATE");  
          }
          else if((ui32BitMask & NEW) != 0)
          {
              strcpy(pachRepBuff , "NEW");      
          }    
          strcat(pachRepBuff , DELIMITER);
          snprintf(achBuff, sizeof(uint64),"%5ld",gstConfigs.ui64ServID);
          strcat( pachRepBuff, achBuff);
          memset(achBuff , 0 , sizeof(achBuff));
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

     if((ui32BitMask & CLIENT) != 0)
     {
        if((bytes = sendto(gUDPCliSockFD,pachRepBuff,strlen(pachRepBuff),
                0,(struct sockaddr *)pstRcvdMsg->strcvd_addr,size)) < 0)
        {
           FINDME_LOG("ERROR : Message could not be sent to client\n");
           perror("Sendto");
        }
     }
     else if((ui32BitMask & SERVER) != 0)
     {
        SendtoServ(pachRepBuff);           
     }
     freeMsg(pstRcvdMsg);
     free(pachRepBuff);
  }
 
}


void SendtoServ(int8 * pachRepBuff)
{
  struct sockaddr_in6 addr_ipv6 = {0};
  int8 achAddr[2 * MAX_LINE_LENGTH] = {0};

    if(strcnt(gstConfigs.achServerIP,':') <= 1)
   {
       MapV4toV6(achAddr);
   }
   else
   {
      memcpy(achAddr , gstConfigs.achServerIP , strlen(gstConfigs.achServerIP));
      achAddr[strlen(achAddr)-1] = '\0';
   }

  setAddrIpv6(&addr_ipv6,gstConfigs.ui32Port,achAddr);

  if((sendto(gUDPServSockFD,pachRepBuff,strlen(pachRepBuff),
          0,(struct sockaddr *)&addr_ipv6,sizeof(struct sockaddr_in6))) < 0)
  {
     FINDME_LOG("ERROR : Message could not be sent to client\n");
     perror("Sendto");
  }


}

void HandleCliQuery(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr,uint64 ui64ID)
{
  /*Start decoding after the message type field*/

 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 int8 * pi8Value = NULL;
 /*uint64 ui64RecId = 0;*/
 uint32 ui32BitMask = 0;
 uint32 ui32QueryMask = 0;
 stRecord * pstRedAdd = NULL;

 if(NULL != pi8MsgPtr)
 { 
    if(0  == isclient(ui64ID))
    {
         pi8Token = strtok_r(pi8MsgPtr , DELIMITER , &pi8SavePtr);
         pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);

         if(0 == strcmp(pi8Token,"NAME"))
         {
              ui32QueryMask |= NAME;
              ui32BitMask |= EMAIL|ADDRESS|LOCATION|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"EMAIL"))
         {
              ui32QueryMask |= EMAIL;
              ui32BitMask |= NAME|ADDRESS|LOCATION|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"LOCATION"))
         {
              ui32QueryMask |= LOCATION;
              ui32BitMask |= NAME|EMAIL|ADDRESS|CLIENT;
         }
         else if(0 == strcmp(pi8Token,"ADDRESS"))
         {
              ui32QueryMask |= ADDRESS;
              ui32BitMask |= NAME|EMAIL|LOCATION|CLIENT;
         }
         else
         {
              ui32BitMask |= NOTYPE|CLIENT;
             /*Send error message for unknown query*/
         }

         if(0 != ui32QueryMask)
         {
             if(NULL == (pstRedAdd = SearchRecord_New(ui32QueryMask , pi8Value)))
             {
              ui32BitMask = NOTYPE|CLIENT;
             }
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
 uint64 ui64Temp =ui64ID - gstConfigs.ui64ServID; 
/*Check for the valid client ID value*/
   if((ui64Temp >= gi64StartID)  && (ui64Temp < gi64EndID))
   {
      return 0;
   }
return -1;
}

void HandleCliUpdate(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr ,uint64 ui64ID)
{
   stRecord * pstRedAdd = NULL;
   uint32 ui32BitMask = 0;
    /*Search for the record and fill the updated content*/
    if(NULL != (pstRedAdd = SearchRecord(ui64ID)))
    {     
        if((ui32BitMask = FillRecord(pstRedAdd,pi8MsgPtr)) != 0)
        {
           if(pstRedAdd->ui32ContentFull == 0)
            {
              ui32BitMask |= SERVER|NEW|CLIENTID;
              pstRedAdd->ui32ContentFull = 1;
            }
            else
            {
              ui32BitMask |= SERVER|UPDATE|CLIENTID;
            }
        }     
        PrepareCliRsp(pstRcvdMsg,pstRedAdd,ui32BitMask);   
    }
}


int32  HandleClientReceivedMsg(stRcvdMsg * pstRcvdMsg , int8*  pachTestArray , uint64 ui64ID)
{
int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;

if(NULL != pstRcvdMsg)
{

    /*validate client and hendle the client*/
    pi8Token = strtok_r (pachTestArray, DELIMITER , &pi8SavePtr);
    if(0 == (strcmp(pi8Token , "QUERY")))
    {
       HandleCliQuery(pstRcvdMsg , pi8SavePtr,ui64ID);
    }
    else if(0 == (strcmp(pi8Token , "UPDATE")))
    {
       HandleCliUpdate(pstRcvdMsg , pi8SavePtr,ui64ID);
    } 

}	
return 0;
}


int isServer(ui64ID)
{
 
 if(ui64ID >= 10000)
    return 0;

 return -1;
}

uint32 FillRecord(stRecord * pstActRec,int8 * pi8MsgPtr)
{
 int8 * pi8SavePtr = NULL;
 int8 * pi8Token = NULL;
 int8 * pi8Value = NULL;
 stRecord * pstRedAdd = NULL;
 uint32 ui32BitMask = 0;
 pstRedAdd = malloc(sizeof(stRecord));

 /*extract every parameter type and value and store in an temp variable*/
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
                  pstRedAdd->ui64LastUpdate = strtol(pi8Value,NULL,10);
                  ui32BitMask |= TIMESTAMP;
	  }
          pi8Token = strtok_r(NULL , DELIMITER , &pi8SavePtr);
	  pi8Value = strtok_r(NULL , DELIMITER , &pi8SavePtr);
   } 

   /*now,update only if it is latest*/
   if((pstActRec->ui64LastUpdate <  pstRedAdd->ui64LastUpdate) || (pstActRec->ui32ContentFull == 0))
   {
      if((NAME & ui32BitMask)!= 0)
      {
         strcpy(pstActRec->achName, pstRedAdd->achName);
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
    pi8Token = strtok_r(NULL, DELIMITER , &pi8SavePtr);
    if(0 == strcmp("CLIENTID",pi8Token))
    {
        pi8Token = strtok_r(NULL , DELIMITER , &pi8SavePtr);
        ui64RecId = strtol(pi8Token,NULL,10);
        if(NULL == (pstRedAdd = SearchRecord(ui64RecId)))
       {
           /*New record should be created for a new client*/
           pstRedAdd = CreateRecord(ui64RecId);
           if(NULL != pstRedAdd)
           {
               pstRedAdd->ui64RecNum = ui64RecId;
               FillRecord(pstRedAdd,pi8SavePtr);
               pstRedAdd->ui32ContentFull = 0;
           }	           
      }
      else
      {
           /*Client already exists,just update*/      
           FillRecord(pstRedAdd,pi8SavePtr); 
      }
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
     ui64RecId = strtol(pi8Token,NULL,10);

  /*Look for the client record and then update the client*/
     if(NULL != (pstRedAdd = SearchRecord(ui64RecId)))
     {     
        FillRecord(pstRedAdd,pi8SavePtr);
     }
  }
  freeMsg(pstRcvdMsg); 
 }
}

void HandleServerReceivedMsg(stRcvdMsg * pstRcvdMsg,int8*  pachTestArray , uint64 ui64ID)
{

int8 * pi8Token = NULL;
int8 * pi8SavePtr = NULL;

if(NULL !=  pstRcvdMsg)
{
   /*look for key words*/
   pi8Token = strtok_r (pachTestArray, DELIMITER , &pi8SavePtr);
   if(0 == (strcmp(pi8Token , "NEW")))
   {
      printf("IN function = %s\n",__FUNCTION__);
      HandleServNew(pstRcvdMsg , pi8SavePtr,ui64ID);
   }
   else if(0 == (strcmp(pi8Token , "UPDATE")))
   {
      HandleServUpdate(pstRcvdMsg , pi8SavePtr);
   }

}
}
