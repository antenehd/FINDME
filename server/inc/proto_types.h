#ifndef _PROTO
#define _PROTO
void  HandleServUpdate(stRcvdMsg * pstRcvdMsg ,int8 *   pi8MsgPtr);
void  HandleServNew(stRcvdMsg * pstRcvdMsg ,int8 * pi8MsgPtr,uint64 ui64ID);
void SendtoServ(int8 * pachRepBuff);
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
int32  HandleClientReceivedMsg(stRcvdMsg * pstRcvdMsg, int8*  pachTestArray, uint64 ui64ID);
uint32 FillRecord(stRecord * pstActRec,int8 * pi8MsgPtr);
void HandleServerReceivedMsg(stRcvdMsg * pstRcvdMsg, int8*  pachTestArray,uint64 ui64ID);
HashTable_t * CreateHash(int32 i32size);
int32 isServer();
void MapV4toV6( int8 * achAddr);
int strcnt(const int8 *str, int8 c);
int setAddrIpv6(struct sockaddr_in6 *addr_ipv6,uint16_t port,char *ipv6);
int writeConf(char *filename,char *type,char *value);
#endif
