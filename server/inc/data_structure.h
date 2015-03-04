#ifndef _datastructure
#define _datastructure

#define MAX_LINE_LENGTH 255
#define MAX_TYPE_LENGTH 10
#define MAX_MSG_LEN 1024
#define DELIMITER "$"
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
int32 i32NoRec;      
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

typedef enum _msgtypes
{
NAME,
EMAIL,
ADDR,
LOCT,
NOREC,
NOTYPE
}eMsgType;
#endif
