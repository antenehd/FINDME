#ifndef _PROTO
#define _PROTO

stRcvdMsg * CreateMsg();
void ChangeServID();
void RequestServID();
int32 readConfigFile();
void PrintAllRecord();
void sighandler_SIGINT();
void * ProcessThreadStart();
void * SyncThreadStart();
void freeMsg(stRcvdMsg * pstRcvdMsg);
void AssignIDToCli(stRcvdMsg * pstRcvdMsg);
stRecord * SearchRecord(uint64 ui64Id);
void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,stRecord * pstRedAdd,uint32 ui32BitMask);
void HandleCliQuery(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr,uint64 ui64ID);
int32 isclient(uint64 ui64ID);
void HandleCliUpdate(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr ,uint64 ui64ID);
int32  HandleClientReceivedMsg(stRcvdMsg * pstRcvdMsg, int8*  pachTestArray);
uint32 FillRecord(stRecord * pstActRec,int8 * pi8MsgPtr);
void HandleServerReceivedMsg(stRcvdMsg * pstRcvdMsg, int8*  pachTestArray);
HashTable_t * CreateHash(int32 i32size);
int32 isServer();
#endif
