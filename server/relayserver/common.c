#define MAX_STRING	100


int sendMsg(char *msg,int sock,skaddr *addr,int size){
	if(msg && addr){
		if(sendto(sock,msg,MAX_MSG_LEN,0,addr,size)<0){
			perror("send failed");
    	return -1;							
		}
		return 0;
	}
}

//reads the "value" from the "filename"
char * readConf(char *filename,char *value){
	FILE * conf;
	char rd[MAX_STRING];
	char *reply;
	if(filename && value){
		memset(rd,0,MAX_STRING);
		conf=fopen(filename,"r");	
		while(!feof(conf)){
			fscanf(conf,"%s",rd);
			if(strstr(rd,value)!=NULL){
				reply=malloc(sizeof(rd)-strlen(value));
				strcpy(reply,rd+strlen(value));
				return reply;
			}			
		}
		fclose(conf);
	}
	return NULL;
}

//writes the value "value" associated with the type "type" in the file "filename"
int writeConf(char *filename,char *type,char *value){
	FILE * conf;
	char *ptr;
	char rd[MAX_STRING];
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

//assigns server id for new servers
uint16_t assignId(char *srvId){
	char *maxSrvId;
	uint16_t newSrvId;
	maxSrvId=readConf("confrelay","serverId=");
	if(maxSrvId){
		sscanf(maxSrvId,"%hu",&newSrvId);
		free(maxSrvId);
	}
	newSrvId++;
	sprintf(srvId,"%05hu",newSrvId);
	writeConf("confrelay","serverId=",srvId);
	printf("NEW SERVERID : %s\n",srvId);
	return newSrvId;
}

//sends "ACK" messages
int sendAck(char* srvId,int sock,skaddr *addr,int size){
	char reply[strlen(ACK)+strlen(DELIMITER)+strlen(srvId)+1];
	if(srvId && addr){
		memset(reply,0,strlen(ACK)+strlen(DELIMITER)+strlen(srvId)+1);
    strcpy(reply,ACK);
		strcat(reply,DELIMITER);
		strcat(reply,srvId);

		sendMsg(reply,sock,addr,size);
	}
}

//sends "FIN" messages
int sendFin(char* srvId,int sock,skaddr *addr,int size){
	char reply[strlen(FINAL)+strlen(DELIMITER)+strlen(srvId)+1];
	if(srvId && addr){
		memset(reply,0,strlen(FINAL)+strlen(DELIMITER)+strlen(srvId)+1);
    strcpy(reply,FINAL);
		strcat(reply,DELIMITER);
		strcat(reply,srvId);

		sendMsg(reply,sock,addr,size);
	}
}

//pasrses tocken from "msg" at "tockenPosition"
char * parseTocken(char *msg,int tockenPosition,char *delimiter,int size){
	char cpyMsg[size+1];
	char *cpyPtr;
	char *tmpTocken;
	char *tocken;
	if(msg){
		memset(cpyMsg,0,size+1);
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
	if(bind(socket,addr,size)<0)
		perror("bind failed");
}

int setAddrIpv4(skaddr_in *addr_ipv4,uint16_t port,char *ipv4){
	if(addr_ipv4&&ipv4){
		memset(addr_ipv4,0,sizeof(skaddr_in));
		addr_ipv4->sin_family=AF_INET;
		addr_ipv4->sin_port=htons(port);
		if(inet_pton(AF_INET,ipv4,&(addr_ipv4->sin_addr))<0){
			perror("Ipv4 address assignment failed");
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
		if(inet_pton(AF_INET6,ipv6,&(addr_ipv6->sin6_addr))<0){
			perror("Ipv6 address assignment failed");
		}
		return 0;
	}
	return -1;
}

int rcvMsg(int sock,char *msg,skaddr *addr,int size){
	if(recvfrom(sock,msg,MAX_MSG_LEN-1,0,addr,&size)<0){
				perror("Receive failed");	
  }
}
