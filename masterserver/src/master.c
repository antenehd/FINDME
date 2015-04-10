#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "common.h"
#include "master.h" 

extern FILE * fpLog;
extern pthread_mutex_t mtx;
extern pthread_t threadIpv4;
extern pthread_t threadIpv6;
extern uint16_t starSrvId;
extern int socket_ipv4,socket_ipv6; /*Udp sockets for ipv4 and ipv6 server communication*/
extern uint16_t maxNmSrvs;

int savedIpv4; /*stores howmany ipv4 servers joined the relay server*/
int savedIpv6; /*stores howmany ipv6 servers joined the relay server*/
char servIdsIpv4[MAX_NUM_SERVS][SRV_ADDR_LEN];
char servIdsIpv6[MAX_NUM_SERVS][SRV_ADDR_LEN];
skaddr ipv4[MAX_NUM_SERVS]; /*stores ipv4 server addresses*/
skaddr ipv6[MAX_NUM_SERVS]; /*stores ipv6 server addresses*/

/*assigns server id for new server. 'srvId' should be length 6 array*/
void assignId(char *srvId, int type){
	pthread_mutex_lock(&mtx);
	if(type==IPV4 && savedIpv4<maxNmSrvs && starSrvId<65535){
		starSrvId++;
		sprintf(srvId,"%05hu",starSrvId);
		writeConf(CONFILE_SRVID,"serverId=",srvId);	
	}
	else if(type==IPV6 && savedIpv6<maxNmSrvs){
		starSrvId++;
		sprintf(srvId,"%05hu",starSrvId);
		writeConf(CONFILE_SRVID,"serverId=",srvId);	
	}
  else
		strcpy(srvId,NEWSERVER);
	pthread_mutex_unlock(&mtx);
}

/*checks if the server with "srvId" is already exists(saved) and if it exists it returns the index where it is saved other wise it returns the last indx for the array that saves servers and increment "savedIp4/6"*/
int checkSavedSrv(char *srvId,char arrySrvIds[][6],int saved){
	int indx;
 	for(indx=0;indx<saved;indx++){
			if(strcmp(srvId,arrySrvIds[indx])==0)
				return indx;
	}
	return -1;
}

/*saves server's address for future use*/
void saveServ(skaddr *addr,char *srvId,int addrType){
	int indx;
	if(addr && srvId){
		pthread_mutex_lock(&mtx);
		if(addrType==IPV4 && savedIpv4<maxNmSrvs){
			if((indx=checkSavedSrv(srvId,servIdsIpv4,savedIpv4))==-1){
				indx=savedIpv4;
				savedIpv4++;
			}
			memset(&ipv4[indx],0,sizeof(*addr));
			memcpy(&ipv4[indx],addr,sizeof(*addr));
	
			memset(servIdsIpv4[indx],0,SRV_ADDR_LEN);
			strcpy(servIdsIpv4[indx],srvId);
		}
		else if(addrType==IPV6 && savedIpv6<maxNmSrvs){
			if((indx=checkSavedSrv(srvId,servIdsIpv6,savedIpv6))==-1){
				indx=savedIpv6;
				savedIpv6++;
			}
			memset(&ipv6[indx],0,sizeof(*addr));
			memcpy(&ipv6[indx],addr,sizeof(*addr));
			
			memset(servIdsIpv6[indx],0,SRV_ADDR_LEN);
			strcpy(servIdsIpv6[indx],srvId);
		}
		else
			strcpy(srvId,NEWSERVER);
		pthread_mutex_unlock(&mtx);
	}
}

/*deletes serverId */
void delSrvId(char array[][6],int indx,int saved){
	int i;
	printf("arry: %s indx: %d SAVED: %d\n",array[indx],indx,saved);
	if(array){
		for(i=indx;i<(saved-1);i++)
			strcpy(array[i],array[i+1]);		
	}
}

/*deletes serveraddress */
void delSrvAddr(skaddr array[],int indx,int saved){
	int i;
	printf("delet serveaddr %d indx %d\n",saved,indx);
	if(array){
		for(i=indx;i<(saved-1);i++){
			memset(&array[i],0,sizeof(array[i]));
			memcpy(&array[i],&array[i+1],sizeof(array[i+1]));		
		}
	}
}

/*deletes serverId and serveraddress there by deleting the server from the communication*/
void removeServ(char *srvId,int addrType){
	int indx;
	if(srvId){
		pthread_mutex_lock(&mtx);
		if(addrType==IPV4 && ((indx=checkSavedSrv(srvId,servIdsIpv4,savedIpv4))!=-1)){
			delSrvId(servIdsIpv4,indx,savedIpv4);
			delSrvAddr(ipv4,indx,savedIpv4);
			savedIpv4--;
			pthread_mutex_unlock(&mtx);
			return;					
		}  
		else if(addrType==IPV6	&& ((indx=checkSavedSrv(srvId,servIdsIpv6,savedIpv6))!=-1)){
				delSrvId(servIdsIpv6,indx,savedIpv6);
				delSrvAddr(ipv6,indx,savedIpv6);
				savedIpv6--;
				pthread_mutex_unlock(&mtx);
				return;		
		}
		pthread_mutex_unlock(&mtx);
	}
}


/*it extracts servers id from the message 'msg' and store it in 'strSrvId'.'strSrvId' is array of len 6*/
void getSrvId(char *msg,char *strSrvId){
	char *srvId;
	if(msg && strSrvId){
		if((srvId=parseTocken(msg,2,DELIMITER))!=NULL){
			if(strlen(srvId)==(SRV_ADDR_LEN-1)){
				strcpy(strSrvId,srvId);
				free(srvId);
				return;
			}
			free(srvId);
		}
		strcpy(strSrvId,BAD_SRV_ID);
	}		
}

/*sends update messages to other servers, excluding the one who send the update message*/
void updateSrvs(char *msg,char *srvId){
	int i;
	int excluded=0;
	if(msg){
		pthread_mutex_lock (&mtx);
		printf("sending updates for serv : %s \n",msg);
		for(i=0;i<savedIpv4;i++){
			if(excluded == 1 || strcmp(servIdsIpv4[i],srvId)!=0){
				printf("sending updates for IPV4 SRV : %s \n",servIdsIpv4[i]);
				sendMsg(msg,socket_ipv4,&ipv4[i],sizeof(skaddr_in));
			}	
			else
				excluded=1;
		}
		pthread_mutex_unlock (&mtx);
	
		/*for ipv6 addressed servers*/
		pthread_mutex_lock(&mtx);
		for(i=0;i<savedIpv6;i++){
			if(excluded == 1 || strcmp(servIdsIpv6[i],srvId)!=0){
				printf("sending updates for IPV6 SRV : %s \n",servIdsIpv6[i]);
				sendMsg(msg,socket_ipv6,&ipv6[i],sizeof(skaddr_in6));
			}
			else
				excluded=1;
		}			
		pthread_mutex_unlock (&mtx);
	}	
}	

/*handles incoming messages */
void  handlMsg(char *msg, int sock, skaddr *addr,int size,int type){
	char *msgType;
	char srvId[SRV_ADDR_LEN];
	time_t currTime;

	currTime = time(NULL);
	printf("message received %s\n",msg);
	memset(srvId,0,SRV_ADDR_LEN);
	if(msg && addr){

		msgType=parseTocken(msg,1,DELIMITER);
		getSrvId(msg,srvId);
		
		if(msgType && (strcmp(srvId,BAD_SRV_ID)!=0)){
			if(strcmp(msgType,UPDATE)==0){
				updateSrvs(msg,srvId);			
			}
			else if(strcmp(msgType,JOIN)==0){
				if(strcmp(srvId,NEWSERVER)==0){
					assignId(srvId,type); printf("assignid %s\n",srvId);
				}
				if(strcmp(srvId,NEWSERVER)!=0){
					saveServ(addr,srvId,type);
					LOG("%ld : Server %s joined.\n",currTime,srvId);
				}
				
				setupAndSendMsg(srvId,JOIN,sock,addr,size);
			}
			else if(strcmp(msgType,DSJOIN)==0 && (strcmp(srvId,BAD_SRV_ID)!=0)){
				printf("disjoin %s\n",srvId);
				removeServ(srvId,type);				
				setupAndSendMsg(srvId,DSJOIN,sock,addr,size);
				LOG("%ld : Server %s disjoined.\n",currTime,srvId);
			}
			free(msgType);
		}
	}  
}

/*setup signal handling*/
void sigAction(int signal,struct sigaction *sigact,void handler (int)){
	memset(sigact,0,sizeof(*sigact));
	sigact->sa_handler=handler;
	sigaction(signal,sigact,NULL);
}

/*handles signals */
void  signalHandler(int sigNumber){
	time_t currTime;

	/*handles when ipv4 and ipv6 threads receive signal*/
	if(sigNumber==SIGUSR1){
		pthread_exit(NULL);
		return;
	}

	currTime = time(NULL);
	LOG("%ld : Signal received, signal number: %d\n",currTime,sigNumber);

	if(threadIpv4!=0){
		if(pthread_kill(threadIpv4,SIGUSR1)!=0)
			LOG("	Sending kill signal to ipv4 thread failed\n");
	}
	if(threadIpv6!=0){
		if(pthread_kill(threadIpv6,SIGUSR1)!=0)
			LOG("	Sending kill signal to ipv6 thread failed\n");
	}
}
/*checks for incoming ipv4 server messages*/
void * ipv4Msgs(){
	char msg[MAX_MSG_LEN];
	skaddr_in rmtAddrIpv4;
	int sizev4=sizeof(skaddr_in);
	struct sigaction sig;
	time_t currTime;
	
	savedIpv4=0;
	currTime = time(NULL);
	LOG("%ld : Ipv4 thread started.\n",currTime);	

	sigAction(SIGUSR1,&sig,signalHandler);
	sigAction(SIGPIPE,&sig,signalHandler);

	for(;;){
 		memset(msg,0,MAX_MSG_LEN);
		memset(&rmtAddrIpv4,0,sizev4);
		rcvMsg(socket_ipv4,msg,(skaddr*)&rmtAddrIpv4,(socklen_t)sizev4);
		handlMsg(msg,socket_ipv4,(skaddr *)&rmtAddrIpv4,sizev4,IPV4);
	}
}

/*checks for incoming ipv6 server messages*/
void * ipv6Msgs(){
	char msg[MAX_MSG_LEN];
	skaddr_in6 rmtAddrIpv6;
	int sizev6=sizeof(skaddr_in6);
	struct sigaction sig;
	time_t currTime;

	savedIpv6=0;
	currTime = time(NULL);
	LOG("%ld : Ipv6 thread started.\n",currTime);

	sigAction(SIGUSR1,&sig,signalHandler);
	sigAction(SIGPIPE,&sig,signalHandler);
	
	for(;;){
 		memset(msg,0,MAX_MSG_LEN);
		memset(&rmtAddrIpv6,0,sizev6);
		rcvMsg(socket_ipv6,msg,(skaddr *)&rmtAddrIpv6,(socklen_t)sizev6);
		handlMsg(msg,socket_ipv6,(skaddr *)&rmtAddrIpv6,sizev6,IPV6);   
	}
}


int copyIpAddr(char *arg,char *ipAddr,int size){
	if(arg && ipAddr && strlen(arg)<=size){
		strcpy(ipAddr,arg);
		return 0;
	}
	printf("Invalid ip address given\n");
	return -1;
}

int copyPort(char *arg,uint16_t *port){
	int temp;	
	if(arg && port && strlen(arg)<=5){
		if((sscanf(arg,"%d",&temp)==1) && temp<=65535){
			*port=(uint16_t)temp;
			return 0;
		}			
	}
	printf("Invalid port number given\n");
	return -1;
}

void cmdUsage(){
	printf("Usage: mserver [option] [value]\n");
	printf("option:\n -ipv4   for ip4 address\n");
	printf(" -ipv6   for ipv6 address\n");
	printf(" -port   to specify port number\n -mns    maximum number of servers supported(less than 100)");
	printf("e.g. mserver -ipv4 127.0.01 -ipv6 ::01 -port 5002 -mns 20\n");
}

int parseCmdArg(int arg,char *argv[],char *ipv4,char *ipv6,uint16_t *port,uint16_t *mxSrvs){
	int i;
	for(i=1;(i+1)<arg;i+=2){
		if(strcmp(argv[i],"-ipv4")==0){
			if(copyIpAddr(argv[i+1],ipv4,SIZE_IPV4)==-1)
				return -1;
		}
		else if(strcmp(argv[i],"-ipv6")==0){
			if(copyIpAddr(argv[i+1],ipv6,SIZE_IPV6)==-1)
				return -1;
		}
		else if(strcmp(argv[i],"-port")==0){
			if(copyPort(argv[i+1],port)==-1)
				return -1;
		}
		else if(strcmp(argv[i],"-mns")==0){
			if(copyPort(argv[i+1],mxSrvs)==-1)
				return -1;
		}
		else{
			cmdUsage();
			return -1;
		}
	}
	if(*port==0){
		printf("port number is missing\ncommand: mserver -port <port number>\n");
		return -1;
	}
	if((*mxSrvs==0) || (*mxSrvs>100)){
		printf("Maximum number of servers is missing\ncommand: mserver -mns <number of servers> and it should not be more than 100\n");
		return -1;
	}
	if((strlen(ipv4)==0) && (strlen(ipv6)==0)){
		printf("Ip address should be provided\ncommand: mserver -ipv4(-ipv6) <ipaddress>\n");
		return -1;
	}
	return 0;
}

int readAddr(char *ipv4,char *ipv6,uint16_t *port,uint16_t *mxSrvs){
	char *tempIpv4,*tempIpv6,*tempPort,*tempMxSrvs;

	if(ipv4 && ipv6 && port){

		tempIpv4=readConf(CONFILE_ADDR,"ipv4=");
		if(tempIpv4){
			copyIpAddr(tempIpv4,ipv4,SIZE_IPV4);
			free(tempIpv4);
		}

		tempIpv6=readConf(CONFILE_ADDR,"ipv6=");
		if(tempIpv6){
			copyIpAddr(tempIpv6,ipv6,SIZE_IPV6);
			free(tempIpv6);
		}

		tempPort=readConf(CONFILE_ADDR,"port=");
		if(tempPort){
			copyPort(tempPort,port);
			free(tempPort);
		}

		tempMxSrvs=readConf(CONFILE_ADDR,"maxsrvs=");
		if(tempMxSrvs){
			copyPort(tempMxSrvs,mxSrvs);
			free(tempMxSrvs);
		}

		if(*port==0){
			printf("Port number should be configure in \"confadd\" file as: port=<portnumber>\n");
			return -1;
		}
		if((*mxSrvs==0) || (*mxSrvs>100)){
			printf("Maximum number of severs that can be served should be configure in \"confadd\" file as: maxsrvs=<maximumnumberofservers> and it should not be more than 100\n");
			return -1;
		}
		if(strlen(ipv4)==0 &&strlen(ipv6)==0){
			printf("Ip addresses should be configured in \"confadd\" file as: ipv4=<ipv4addr> ipv6=<ipv6addr>\n");
			return -1;
		}
		return 0;
	}
	return -1;
}

int readStarSrvId(){
	char *srvId;
	srvId=readConf(CONFILE_SRVID,"serverId=");
	if(srvId){
		sscanf(srvId,"%hu",&starSrvId);
		free(srvId);
		return 0;
	}
	return -1;
}


