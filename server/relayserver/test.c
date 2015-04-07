#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_SRV_CONN 20
#define DELIMITER "$"
#define ACK "NEW"
#define UPDATE "UPDATE"
#define IPV4 4
#define IPV6 6
#define LCL_PORT 5002
#define RMT_PORT 5003
#define LCL_IPV4 "127.0.0.1"
#define LCL_IPV6 "::1"
#define MAX_MSG_LEN 1024
#define NEWSERVER "00000"
#define FINAL "FIN"
typedef struct sockaddr skaddr;
typedef struct sockaddr_in skaddr_in;
typedef struct sockaddr_in6 skaddr_in6;
int socket_ipv4,socket_ipv6;

#include "common.c"

void *testIpv4(){
	int i;
	skaddr_in rmt_ipv4;
	char *ip;
	char srvId[6];
	char msg[MAX_MSG_LEN];
	char *confile="confipv4";
	int sizev4=sizeof(skaddr_in);
	char *tocken=NULL;
	char *data="CLINTID$323233$LOCATION$234324\n";
	char update[strlen(data)+14];

	//read the conf file for relay ip address
	printf("IPV4 readign conf file for relay ip address\n");
	ip=readConf("confipv4","ip=");
  
	//setup romote address
  if(ip){
		printf("IPV4 setting up remote address relayip = %s\n",ip); 
		setAddrIpv4(&rmt_ipv4,RMT_PORT,ip);
		free(ip);
		//printf("remote port address= %hu\n",ntohs(rmt_ipv4->sin_port));
	}

	else{
			perror("IPV4 relay ip can not be found\n");
			pthread_exit(NULL);
	}

	//send first ack as new server
	printf("IPV4 sending ACK as new server\n");
	sendAck(NEWSERVER,socket_ipv4,(skaddr *)&rmt_ipv4,sizev4);

	//waite for server response ACK
	printf("IPV4 waite for server response ACK\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv4,msg,NULL,sizev4);

	//write server id
	printf("IPV4 relay server response %s\n",msg);
	tocken=parseTocken(msg,1,DELIMITER,strlen(msg));
	if(tocken==NULL || (strcmp(tocken,ACK)!=0)){
		//update could be received before ACK
		perror("Relay server response incorrect\n");
		free(tocken);
		pthread_exit(NULL);
	}

	writeConf("confipv4","serverId=",msg+strlen(ACK)+strlen(DELIMITER));
	memset(srvId,0,6);	
	memcpy(srvId,msg+strlen(ACK)+strlen(DELIMITER),5);

	//test sending updates
	memset(update,0,sizeof(update));
	strcpy(update,UPDATE);
	strcat(update,DELIMITER);
	strcat(update,srvId);
	strcat(update,DELIMITER);
	strcat(update,data);
	printf("IPV4 sending update message= %s\n",update);
	sendMsg(update,socket_ipv4,(skaddr *)&rmt_ipv4,sizev4);

	//receive updates,this would hang if this thread started later
	printf("IPV4 receive update message\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv4,msg,NULL,sizev4);
	printf("IPV4 update message received = %s\n",msg);

	//sending final
	printf("IPV4 sending FINAL : \n");
	sendFin(srvId,socket_ipv4,(skaddr *)&rmt_ipv4,sizev4);
	
	//waite for server response down
	printf("IPV4 waite for server response FINAL\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv4,msg,NULL,sizev4);
	printf("IPV4 FINAL received : %s\n",msg);
	pthread_exit(NULL);
}


void *testIpv6(){
	int i;
	skaddr_in6 rmt_ipv6;
	char srvId[6];
	char *ip;
	char msg[MAX_MSG_LEN];
	char *confile="confipv6";
	int sizev6=sizeof(skaddr_in6);
	char *tocken=NULL;
	char *data="CLINTID$000043$LOCATION$234234\n";
	char update[strlen(data)+14];

	//read the conf file for relay ip address
	printf("ipv6 readign conf file for relay ip address ipv6\n");
	ip=readConf("confipv6","ip=");
  
	//setup romote address
  if(ip){
		printf("ipv6 setting up remote address relayipv6 = %s\n",ip); 
		setAddrIpv6(&rmt_ipv6,RMT_PORT,ip);
		free(ip);
		//printf("remote port address= %hu\n",ntohs(rmt_ipv4->sin_port));
	}

	else{
			perror("ipv6 relay ip can not be found\n");
			pthread_exit(NULL);
	}

	//send first ack as new server
	printf("ipv6 sending ACK as new server\n");
	sendAck(NEWSERVER,socket_ipv6,(skaddr *)&rmt_ipv6,sizev6);

	//waite for server response ACK,
	printf("ipv6 waite for server response ACK\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv6,msg,NULL,sizev6);

	//write server id
	printf("ipv6 relay server response %s\n",msg);
	tocken=parseTocken(msg,1,DELIMITER,strlen(msg));
	if(tocken==NULL || (strcmp(tocken,ACK)!=0)){
		//update could be received before ACK
		perror("IPV6 Relay server response incorrect\n");
		free(tocken);
		pthread_exit(NULL);
	}

	writeConf("confipv6","serverId=",msg+strlen(ACK)+strlen(DELIMITER));
	memset(srvId,0,6);	
	memcpy(srvId,msg+strlen(ACK)+strlen(DELIMITER),5);

	//test sending updates
	memset(update,0,sizeof(update));
	strcpy(update,UPDATE);
	strcat(update,DELIMITER);
	strcat(update,srvId);
	strcat(update,DELIMITER);
	strcat(update,data);
	printf("ipv6 sending update message= %s\n",update);
	sendMsg(update,socket_ipv6,(skaddr *)&rmt_ipv6,sizev6);

	//receive updates,this would hang if there is nothing to be received
	printf("ipv6 receive update message\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv6,msg,NULL,sizev6);
	printf("ipv6 update message received = %s\n",msg);
	

	//sending server down 
	printf("IPV6 sending FINAL\n");
	sendFin(srvId,socket_ipv6,(skaddr *)&rmt_ipv6,sizev6);
	
	//waite for server response down
	printf("IPV6 waite for server response FINAL\n");
	memset(msg,0,MAX_MSG_LEN);
	rcvMsg(socket_ipv6,msg,NULL,sizev6);
	printf("IPV6  FINAL received : %s\n",msg);
	pthread_exit(NULL);
}
int main(){
	skaddr_in addr_ipv4;
	skaddr_in6 addr_ipv6;	
	
	pthread_t thread_ipv4,thread_ipv6;	

	//pthread_mutex_init(&mtx, NULL);

	socket_ipv4=creatUdpSocketIpv4();
	socket_ipv6=creatUdpSocketIpv6();

	setAddrIpv4(&addr_ipv4,LCL_PORT,LCL_IPV4);
	setAddrIpv6(&addr_ipv6,LCL_PORT,LCL_IPV6);

	bindSock(socket_ipv4,(skaddr*)&addr_ipv4,sizeof(addr_ipv4));
  bindSock(socket_ipv6,(skaddr*)&addr_ipv6,sizeof(addr_ipv6));
 	
	//set_addrPtr_null();
	
	pthread_create(&thread_ipv4,NULL, testIpv4, NULL);
	pthread_create(&thread_ipv6,NULL, testIpv6, NULL);
	
	pthread_join(thread_ipv4,NULL);
  pthread_join(thread_ipv6,NULL);
	
	//pthread_mutex_destroy(&mtx);

}
