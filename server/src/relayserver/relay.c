#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define LCL_PORT 5003
#define LCL_IPV4 "127.0.0.1"
#define LCL_IPV6 "::1"
#define MAX_SRV_CONN 20
#define SRV_ADDR_LEN 6
#define DELIMITER "$"
#define ACK "NEW"
#define UPDATE "UPDATE"
#define IPV4 4
#define IPV6 6
#define MAX_MSG_LEN 1024
#define NEWSERVER "00000"
#define FINAL "FIN"
//#define MAX(x,y) (x>=y) ? x : y

typedef struct sockaddr skaddr;
typedef struct sockaddr_in skaddr_in;
typedef struct sockaddr_in6 skaddr_in6;

pthread_mutex_t mtx;
skaddr ipv4[MAX_SRV_CONN]; //stores ipv4 server addresses
skaddr ipv6[MAX_SRV_CONN]; //stores ipv6 server addresses
char servIds_ipv4[MAX_SRV_CONN][SRV_ADDR_LEN];
char servIds_ipv6[MAX_SRV_CONN][SRV_ADDR_LEN];
int savedIpv4=0; //stores howmany ipv4 servers joined the relay server
int savedIpv6=0; //stores howmany ipv6 servers joined the relay server

int socket_ipv4,socket_ipv6; //Udp sockets for ipv4 and ipv6 server communication

#include "common.c"  //this will change in final implemention

//saves servers' address for future use
int saveServ(skaddr *addr,char *srvId,int addrType){
	pthread_mutex_lock(&mtx);
	if(addrType==IPV4){
		memset(&ipv4[savedIpv4],0,sizeof(*addr));
		memcpy(&ipv4[savedIpv4],addr,sizeof(*addr));

		memset(servIds_ipv4[savedIpv4],0,SRV_ADDR_LEN);
		strcpy(servIds_ipv4[savedIpv4],srvId);

		savedIpv4++;
	}
	else{
		memset(&ipv6[savedIpv6],0,sizeof(*addr));
		memcpy(&ipv6[savedIpv6],addr,sizeof(*addr));
		
		memset(servIds_ipv6[savedIpv6],0,SRV_ADDR_LEN);
		strcpy(servIds_ipv6[savedIpv6],srvId);

		savedIpv6++;
	}
	pthread_mutex_unlock(&mtx);
}

//deletes serverId 
int delSrvId(char **array,int indx,int saved){
	for(indx;indx<(saved-1);indx++)
		strcpy(array[indx],array[indx+1]);		
}

//deletes serveraddress 
int delSrvAddr(skaddr array[],int indx,int saved){
	for(indx;indx<(saved-1);indx++){
		memset(&array[indx],0,sizeof(array[indx]));
		memcpy(&array[indx],&array[indx+1],sizeof(array[indx+1]));		
	}
}

//deletes serverId and serveraddress there by deleting the server from the communication
int removeServ(char *srvId){
	int indx;
	if(srvId){
		pthread_mutex_lock(&mtx);
		for(indx=0;indx<savedIpv4;indx++){
			if(strcmp(servIds_ipv4[indx],srvId)==0){
				delSrvId((char **)servIds_ipv4,indx,savedIpv4);
				delSrvAddr(ipv4,indx,savedIpv4);
				pthread_mutex_unlock(&mtx);
				savedIpv4--;
				return 0;
			}				
		}
	
		for(indx=0;indx<savedIpv6;indx++){
			if(strcmp(servIds_ipv6[indx],srvId)==0){
				delSrvId((char **)servIds_ipv6,indx,savedIpv6);
				delSrvAddr(ipv6,indx,savedIpv6);
				savedIpv6--;
				pthread_mutex_unlock(&mtx);
				return 0;
			}		
		}
		pthread_mutex_unlock(&mtx);
	}
}

/*it extracts servers address from the message if the server is new server it assignes new server address for the server*/
void getSrvId(char *msg,char *strSrvId,int type){
	char *srvId;
	if(type==IPV4 && savedIpv4<MAX_SRV_CONN){
	  srvId=parseTocken(msg,2,DELIMITER,strlen(msg));
		if(srvId){
			if(strcmp(srvId,NEWSERVER)==0)
				assignId(srvId);
							
			strcpy(strSrvId,srvId);
			free(srvId);
		}
	}			
	else if(type==IPV6 && savedIpv6<MAX_SRV_CONN){
	
		srvId=parseTocken(msg,2,DELIMITER,strlen(msg));
		if(srvId){
			if(strcmp(srvId,NEWSERVER)==0)
				assignId(srvId);
							
			strcpy(strSrvId,srvId);
			free(srvId);
		}
	}
	else
		strcpy(strSrvId,NEWSERVER);	
}

/*sends update messages to other servers, excluding the one who send the update message*/
void updateSrvs(char *msg,char *srvId){
	int i;
	int excluded=0;
	if(msg){
		pthread_mutex_lock (&mtx);
		printf("sending updates for serv : %s \n",msg);
		for(i=0;i<savedIpv4;i++){
			if(excluded == 1 || strcmp(servIds_ipv4[i],srvId)!=0){
				printf("sending updates for IPV4 SRV : %s \n",servIds_ipv4[i]);
				sendMsg(msg,socket_ipv4,&ipv4[i],sizeof(skaddr_in));
			}	
			else
				excluded=1;
		}
		pthread_mutex_unlock (&mtx);
	
		//for ipv6 addressed servers
		pthread_mutex_lock(&mtx);
		for(i=0;i<savedIpv6;i++){
			if(excluded == 1 || strcmp(servIds_ipv6[i],srvId)!=0){
				printf("sending updates for IPV6 SRV : %s \n",servIds_ipv6[i]);
				sendMsg(msg,socket_ipv6,&ipv6[i],sizeof(skaddr_in6));
			}
			else
				excluded=1;
		}			
		pthread_mutex_unlock (&mtx);
	}	
}	

//handles incoming messages 
void  handlMsg(char *msg, int sock, skaddr *addr,int size,int type){
	char *msgType;
	char srvId[SRV_ADDR_LEN];
	memset(srvId,0,SRV_ADDR_LEN);
	if(msg && addr){
		msgType=parseTocken(msg,1,DELIMITER,strlen(msg));
		getSrvId(msg,srvId,type);
		if(msgType){
			if(strcmp(msgType,UPDATE)==0){
				updateSrvs(msg,srvId);			
			}
			else if(strcmp(msgType,ACK)==0){
				if(strcmp(srvId,NEWSERVER)!=0)
					saveServ(addr,srvId,type);
				sendAck(srvId,sock,addr,size);
			}
			else if(strcmp(msgType,FINAL)==0){
				removeServ(srvId);				
				sendFin(srvId,sock,addr,size);
			}
			free(msgType);
		}
	}  
}

//checks for incoming ipv4 server messages
void * Ipv4_msgs(){
	char msg[MAX_MSG_LEN];
	skaddr_in rmtAddr_ipv4;
	int sizev4=sizeof(skaddr_in);

	for(;;){
 		memset(msg,0,MAX_MSG_LEN);
		memset(&rmtAddr_ipv4,0,sizev4);
		rcvMsg(socket_ipv4,msg,(skaddr*)&rmtAddr_ipv4,sizev4);
		handlMsg(msg,socket_ipv4,(skaddr *)&rmtAddr_ipv4,sizev4,IPV4);   //TODO: will be implemented with threads
	}
}

//checks for incoming ipv6 server messages
void * Ipv6_msgs(){
	char msg[MAX_MSG_LEN];
	skaddr_in6 rmtAddr_ipv6;
	int sizev6=sizeof(skaddr_in6);

	for(;;){
 		memset(msg,0,MAX_MSG_LEN);
		memset(&rmtAddr_ipv6,0,sizev6);
		rcvMsg(socket_ipv6,msg,(skaddr *)&rmtAddr_ipv6,sizev6);
		handlMsg(msg,socket_ipv6,(skaddr *)&rmtAddr_ipv6,sizev6,IPV6);   //TODO: will be implemented with threads
	}
}

int main(){

	skaddr_in addr_ipv4;
	skaddr_in6 addr_ipv6;	
	pthread_t thread_ipv4,thread_ipv6;	

	pthread_mutex_init(&mtx, NULL);

	socket_ipv4=creatUdpSocketIpv4();
	socket_ipv6=creatUdpSocketIpv6();

	setAddrIpv4(&addr_ipv4,LCL_PORT,LCL_IPV4);
	setAddrIpv6(&addr_ipv6,LCL_PORT,LCL_IPV6);

	bindSock(socket_ipv4,(skaddr*)&addr_ipv4,sizeof(addr_ipv4));
  bindSock(socket_ipv6,(skaddr*)&addr_ipv6,sizeof(addr_ipv6));
 		
	pthread_create(&thread_ipv4,NULL, Ipv4_msgs, NULL);
	pthread_create(&thread_ipv6,NULL, Ipv6_msgs, NULL);
	
	pthread_join(thread_ipv4,NULL);
  pthread_join(thread_ipv6,NULL);
	
	pthread_mutex_destroy(&mtx);
}

