#ifndef _PROTO
#define _PROTO

void * ProcessThreadStart();
void * SyncThreadStart();
void freeMsg(stRcvdMsg * pstRcvdMsg);
void AssignIDToCli(stRcvdMsg * pstRcvdMsg);
stRecord * SearchRecord(uint64 ui64Id);
void PrepareCliRsp(stRcvdMsg * pstRcvdMsg,int32 MsgCode);
void HandleCliQuery(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr,uint64 ui64ID);
int32 isclient(uint64 ui64ID);
void HandleCliUpdate(stRcvdMsg * pstRcvdMsg , int8 * pi8MsgPtr ,uint64 ui64ID);
int32  HandleClientReceivedMsg(stRcvdMsg * pstRcvdMsg);

#endif
