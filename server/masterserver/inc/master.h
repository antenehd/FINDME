#ifndef _MASTER
#define _MASTER

#define SIZE_IPV4 15
#define SIZE_IPV6 40
#define MAX_SRV_CONN 20
#define SRV_ADDR_LEN 6
#define ACK "NEW"
#define UPDATE "UPDATE"
#define IPV4 4
#define IPV6 6
#define MAX_MSG_LEN 1024
#define NEWSERVER "00000"
#define FINAL "FIN"
#define CONFILE_ADDR "address.conf"
#define CONFILE_SRVID "servid.conf"
#define LOG_FILE	"fdmaster.log"
#define LOG(...) {if(fpLog) fprintf(fpLog, __VA_ARGS__);}

void assignId(char *srvId);
int checkSavedSrv(char *srvId,char arrySrvIds[][6],int saved);
void saveServ(skaddr *addr,char *srvId,int addrType);
void delSrvId(char array[][6],int indx,int saved);
void delSrvAddr(skaddr array[],int indx,int saved);
void removeServ(char *srvId,int addrType);
void getSrvId(char *msg,char *strSrvId,int type);
void updateSrvs(char *msg,char *srvId);
void  handlMsg(char *msg, int sock, skaddr *addr,int size,int type);
void sigAction(int signal,struct sigaction *sigact,void handler (int));
void  signalHandler(int sigNumber);
void * Ipv4Msgs();
void * Ipv6Msgs();
int copyIpAddr(char *arg,char *ipAddr,int size);
int copyPort(char *arg,uint16_t *port);
void cmdUsage();
int parseCmdArg(int arg,char *argv[],char *ipv4,char *ipv6,uint16_t *port);
int readAddr(char *ipv4,char *ipv6,uint16_t *port);
int readMaxSrvId();

#endif
