#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

struct sockaddr *mstrAddr;
int updSock; //socket for update comm with master

/*this function is to be called in a thread in main function when the server starts.This function handles the first communication with master server to tell the master server it is up and to ask for server id if it does not have any yet*/

void * helloMaster(){
	int sock,srvId;
	unsigned char sync,ack;
	char *ipv4,*ipv6;
	int version; 			//master server ipaddr version, 4 for ipv4  and 6 for ipv6
	struct sockaddr_in *addr_ipv4;
	struct sockaddr_in6 *addr_ipv6;
	struct sockaddr *addr;
	size_t sizev4,sizev6,size;
	FILE * strUpConf;

	strUpConf=fopen("config","r");
	fscanf(strUpConf,"%d",&version);
		printf("seg1\n");
	if(version==4){
		ipv4=malloc(15);
		memset(ipv4,0,15);
		fscanf(strUpConf,"%s",ipv4);
		
		sizev4=sizeof(struct sockaddr_in);
		addr_ipv4=malloc(sizev4);
		memset(addr_ipv4,0,sizev4);
		addr_ipv4->sin_family=AF_INET;
		addr_ipv4->sin_port=htons(5001);
		if(inet_pton(AF_INET,ipv4,&addr_ipv4->sin_addr)<0){
			perror("Ipv4 address assignment failed");
		}

		addr=(struct sockaddr*)addr_ipv4;
		size=sizev4;
		
		if((sock=socket(AF_INET,SOCK_DGRAM,0))<0){
			perror("socket creation for master comm failed");
		}
		if(bind(sock,addr,sizev4)<0){
			perror("bind creation for master comm failed");
		}
	}
	
	else if(version==6){
		ipv6=malloc(39);
		memset(ipv6,0,39);
		fscanf(strUpConf,"%s",ipv6);

		sizev6=sizeof(struct sockaddr_in6);
		addr_ipv6=malloc(sizev6);
		memset(addr_ipv6,0,sizev6);
		addr_ipv6->sin6_family=AF_INET6;
		addr_ipv6->sin6_port=htons(5001);
		if(inet_pton(AF_INET6,ipv6,&addr_ipv6->sin6_addr)<0){
			perror("Ipv6 address assignment failed");
		}

		addr=(struct sockaddr*)addr_ipv6;
		size=sizev6;

	  if((sock=socket(AF_INET6,SOCK_DGRAM,0))<0){
			perror("socket creation for master comm failed");
		}
		if(bind(sock,addr,sizev6)<0){
			perror("bind creation for master comm failed");
		}
	}
  
	//read server id and fill sync
	srvId=0;
	fscanf(strUpConf,"%d",&srvId);
	sync=(unsigned char)srvId;
	fclose(strUpConf);
	
	//send keepalive(i am up) message
	if(sendto(sock,&sync,1,0,addr,size)<0){
		perror("sending sync failed");
	}

	//receive ack
	if(recvfrom(sock,&ack,1,0,NULL,NULL)<0){
		perror("receiving ack failed");
	}

	if(sync==0){
		srvId=(int)ack;
		strUpConf=fopen("config","a");
		fprintf(strUpConf,"%d",srvId);
		fclose(strUpConf);
	}
	
	//create and save socket in global variables for updates
	if(version==4){
		addr_ipv4->sin_port=htons(5000);
		if((updSock=socket(AF_INET,SOCK_DGRAM,0))<0){
			perror("socket creation for master comm failed");
		}
		if(bind(updSock,addr,sizev4)<0){
			perror("bind creation for master comm failed");
		}		
	}
	else if(version==6){
		addr_ipv6->sin6_port=htons(5000);
		if((updSock=socket(AF_INET6,SOCK_DGRAM,0))<0){
			perror("socket creation for master comm failed");
		}
		if(bind(updSock,addr,sizev6)<0){
			perror("bind creation for master comm failed");
		}
	}

	mstrAddr=addr;
}

int main(){
	helloMaster();
}
