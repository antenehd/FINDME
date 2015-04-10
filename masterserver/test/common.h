#ifndef _COMMON
#define _COMMON
#define MAX_STRING	100 /*maximum word size in a conf file to be read*/
#define MAX_WRITE_STRING	200 /*maximum size of the text in the conf file to be written to*/
#define DELIMITER "$"
#define JOIN_MSG_LEN 13
#define CM_MAX_MSG_LEN 1048

typedef struct sockaddr skaddr;
typedef struct sockaddr_in skaddr_in;
typedef struct sockaddr_in6 skaddr_in6;

int sendMsg(char *msg,int sock,skaddr *addr,int size);
char * readConf(char *filename,char *value);
int writeConf(char *filename,char *type,char *value);
void setupAndSendMsg(char* srvId,char *msgType,int sock,skaddr *addr,int size);
char * parseTocken(char *msg,int tockenPosition,char *delimiter);
int creatUdpSocketIpv4();
int creatUdpSocketIpv6();
int bindSock(int socket,skaddr* addr,int size);
int setAddrIpv4(skaddr_in *addr_ipv4,uint16_t port,char *ipv4);
int setAddrIpv6(skaddr_in6 *addr_ipv6,uint16_t port,char *ipv6);
void rcvMsg(int sock,char *msg,skaddr *addr,socklen_t size);

#endif
