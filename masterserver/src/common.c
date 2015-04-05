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

int sendMsg(char *msg,int sock,skaddr *addr,int size){
	if(msg && addr){
		if(sendto(sock,msg,strlen(msg),0,addr,size)<0){
			perror("send failed");
    	return -1;							
		}
		return 0;
	}
	return -1;
}

/*reads the "value" from the "filename"*/
char * readConf(char *filename,char *value){
	FILE * conf;
	char rd[MAX_STRING];
	char *reply;
	if(filename && value){
		memset(rd,0,MAX_STRING);
		conf=fopen(filename,"r");	
		if(conf){
			while(!feof(conf)){
				fscanf(conf,"%s",rd);
				if(strstr(rd,value)!=NULL){
					reply=malloc(sizeof(rd)-strlen(value));
					strcpy(reply,rd+strlen(value));
					fclose(conf);
					return reply;
				}			
			}
			fclose(conf);
		}
		else
			printf("Reading failed, file \"%s\" does not exist\n",filename);
	}
	return NULL;
}

/*writes the value "value" associated with the type "type" in the file "filename"*/
int writeConf(char *filename,char *type,char *value){
	FILE * conf;
	char *ptr;
	char rd[MAX_WRITE_STRING];
	conf=fopen(filename,"r+");
	memset(rd,0,MAX_STRING);
	fread(rd,sizeof(char),MAX_STRING-1,conf);
	ptr=strstr(rd,type);
	if(ptr){
		memcpy(ptr+strlen(type),value,strlen(value));
		fseek(conf,0,SEEK_SET);
		fwrite(rd,sizeof(char),strlen(rd),conf);
		fclose(conf);
		return 0;
	}
	fclose(conf);
	return -1;
}

/*sends "ACK" and "FIN" messages depending on "msgType"*/
void setupAndSendMsg(char* srvId,char *msgType,int sock,skaddr *addr,int size){
	char reply[SYNC_MSG_LEN];
	if(srvId && addr){
		memset(reply,0,SYNC_MSG_LEN);
    strcpy(reply,msgType);
		strcat(reply,DELIMITER);
		strcat(reply,srvId);

		sendMsg(reply,sock,addr,size);
	}
}

/*parses tocken from "msg" at "tockenPosition"*/
char * parseTocken(char *msg,int tockenPosition,char *delimiter){
	char cpyMsg[CM_MAX_MSG_LEN];
	char *cpyPtr;
	char *tmpTocken;
	char *tocken;
	if(msg){
		memset(cpyMsg,0,CM_MAX_MSG_LEN);
		strcpy(cpyMsg,msg);
		cpyPtr=cpyMsg;
		while(tockenPosition>0){
  		tmpTocken=strtok_r(cpyPtr,delimiter,&cpyPtr);
			tockenPosition--;
		}
    if(tmpTocken){
			tocken=malloc(strlen(tmpTocken)+1);
			memset(tocken,0,strlen(tmpTocken)+1);
			strcpy(tocken,tmpTocken);	
			return tocken;		
		}
	}
	return NULL;
}


int creatUdpSocketIpv4(){
	int sock;
	if((sock=socket(AF_INET,SOCK_DGRAM,0))<0){
			perror("Ipv4 socket creation failed");
			exit(0);
		}
	return sock;
}

int creatUdpSocketIpv6(){
 int sock;
 if((sock=socket(AF_INET6,SOCK_DGRAM,0))<0){
			perror("Ipv6 socket creation failed");
      exit(0);
		}
	return sock;
}

int bindSock(int socket,skaddr* addr,int size){
	if(bind(socket,addr,size)<0){
		perror("bind failed");
		return -1;
	}
	return 0;
}

int setAddrIpv4(skaddr_in *addr_ipv4,uint16_t port,char *ipv4){
	if(addr_ipv4&&ipv4){
		memset(addr_ipv4,0,sizeof(skaddr_in));
		addr_ipv4->sin_family=AF_INET;
		addr_ipv4->sin_port=htons(port);
		if(inet_pton(AF_INET,ipv4,&(addr_ipv4->sin_addr))<=0){
			printf("Ipv4 address assignment failed\n");
			return -1;
		}
		return 0;
	}
	return -1;
}

int setAddrIpv6(skaddr_in6 *addr_ipv6,uint16_t port,char *ipv6){
	if(addr_ipv6&&ipv6){
		memset(addr_ipv6,0,sizeof(skaddr_in6));
		addr_ipv6->sin6_family=AF_INET6;
		addr_ipv6->sin6_port=htons(port);
		if(inet_pton(AF_INET6,ipv6,&(addr_ipv6->sin6_addr))<=0){
			printf("Ipv6 address assignment failed\n");
			return -1;
		}
		return 0;
	}
	return -1;
}

void rcvMsg(int sock,char *msg,skaddr *addr,socklen_t size){
	if(recvfrom(sock,msg,CM_MAX_MSG_LEN-1,0,addr,&size)<0){
				perror("Receive failed\n");	
  }
}
