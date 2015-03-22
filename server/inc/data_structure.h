#ifndef _datastructure
#define _datastructure

#define CLI_PORT_NUM  45000
#define SERV_PORT_NUM 46000

#define MSERVERID "00000001"
#define CONFIG_FILE "./../Findme.conf"
#define LOG_FILE  "./../Log_File.txt"
#define FINDME_LOG(...) {printf(__VA_ARGS__);} 
/* message queue details */
#define MQ_NAME       "/testmsg"
#define MAX_MSG       100
#define MQ_MODE       0777

#define MAX_LINE_LENGTH 255
#define MAX_TYPE_LENGTH 10
#define MAX_MSG_LEN 1024
#define DELIMITER "$"
#define REPLY "REPLY"
#define PRIORITY 1
#define IPV4_ADDR_LEN 15

/*Bit masks for encode and decoding*/
/*Fourth byte for actual value types*/
#define NAME         0x00000001
#define EMAIL        0x00000002
#define ADDRESS      0x00000004
#define LOCATION     0x00000008
#define TIMESTAMP    0x00000010
#define CLIENTID     0x00000020
/*second byte for Error type*/
#define NOTYPE       0x00010000
#define NOREC        0x00020000
/*First byte for client/server identification*/
#define CLIENT       0x01000000
#define SERVER       0x02000000
#define UPDATE       0x04000000
#define NEW          0x08000000

typedef int int32;
typedef unsigned int uint32;
typedef char int8;
typedef unsigned char  uint8;
typedef short int int16;
typedef unsigned short int uint16;
typedef unsigned long int uint64;


/*Data structure to keep the contents of a
record*/
typedef struct _stRecord
{
uint64 ui64RecNum;
uint64 ui64LastUpdate;
uint32 ui32ContentFull;
int8 achName[MAX_LINE_LENGTH];
int8 achEmail[MAX_LINE_LENGTH];
int8 achAddr[MAX_LINE_LENGTH];
int8 achLastKnownLoc[MAX_LINE_LENGTH]; 
}stRecord;


/*Data structure to form message for the message queue*/

typedef struct _stRcvdMsg
{
struct sockaddr *strcvd_addr;
int8 *achBuffer;
}stRcvdMsg;

typedef struct HashRecord_s 
{
  int32 i32key;
  stRecord * pstValue;
  struct HashRecord_s * pstnext;
}HashRecord_t; 

typedef struct HashTable_s
{
 int32 i32size;
 HashRecord_t **pstTable;
}HashTable_t;

typedef struct _stConfigFileItems
{
   int8 achServerIP[MAX_LINE_LENGTH];
   uint64 ui64ServID;
   uint32 ui32Port;
}stConfigFileItems;




#endif
