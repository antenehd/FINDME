#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

struct sockaddr * rly_srvip;
struct sockaddr_in *glb_addrv4;  //should be distroyed in main function exits
struct sockaddr_in6 *glb_addrv6; //should be distroyed in main function exits
int updSock; //socket for update comm with master
const int sizev4=sizeof(struct sockaddr_in);
const int sizev6=sizeof(struct sockaddr_in6);


int creatSocketIpv4(){
	int sock;
	if((sock=socket(AF_INET,SOCK_DGRAM,0))<0){
			perror("socket creation failed");
		}
	return sock;
}

int creatSocketIpv6(){
 int sock;
 if((sock=socket(AF_INET6,SOCK_DGRAM,0))<0){
			perror("socket creation failed");
		}
	return sock;
}

int bindSock(int socket,struct sockaddr* addr,int size){
	if(bind(socket,addr,size)<0)
		perror("bind failed");
}

int setAddrIpv4(struct sockaddr_in *addr_ipv4,uint16_t port,char *ipv4){
	if(addr_ipv4&&ipv4){
		memset(addr_ipv4,0,sizev4);
		addr_ipv4->sin_family=AF_INET;
		addr_ipv4->sin_port=htons(port);
		if(inet_pton(AF_INET,ipv4,&(addr_ipv4->sin_addr))<0){
			perror("Ipv4 address assignment failed");
		}
		return 0;
	}
	return -1;
}

int setAddrIpv6(struct sockaddr_in6 *addr_ipv6,uint16_t port,char *ipv6){
	if(addr_ipv6&&ipv6){
		memset(addr_ipv6,0,sizev6);
		addr_ipv6->sin6_family=AF_INET6;
		addr_ipv6->sin6_port=htons(port);
		if(inet_pton(AF_INET6,ipv6,&(addr_ipv6->sin6_addr))<0){
			perror("Ipv6 address assignment failed");
		}
		return 0;
	}
	return -1;
}



/*this function is to be called in a thread in main function when the server starts.This function handles the first communication with master server to tell the master server it is up and to ask for server id if it does not have any yet*/

int helloRelay(){
	int sock,srvId;
	unsigned char sync,ack;
	char *ipv4,*ipv6;
	int version; 			//master server ipaddr version, 4 for ipv4  and 6 for ipv6
	struct sockaddr_in *addr_ipv4,lcl_ipv4;
	struct sockaddr_in6 *addr_ipv6,lcl_ipv6;
	struct sockaddr *addr;
	size_t size;
	FILE * strUpConf;
	
	strUpConf=fopen("config","r");
	fscanf(strUpConf,"%d",&version);
	glb_addrv4=NULL;
  glb_addrv6=NULL;

	if(version==4){
		
		ipv4=malloc(16);
		memset(ipv4,0,16);
		fscanf(strUpConf,"%s",ipv4);
		printf("ip=%s\n",ipv4);
		addr_ipv4=(struct sockaddr_in*)malloc(sizev4);
		setAddrIpv4(addr_ipv4,5001,ipv4);  //to be saved in global variable for sending updatas
		
		addr=(struct sockaddr*)addr_ipv4;
		size=sizev4;
		
		memset(&lcl_ipv4,0,sizev4);
		setAddrIpv4(&lcl_ipv4,5003,"127.0.0.1"); //for sync
		sock=creatSocketIpv4();		
		bindSock(sock,(struct sockaddr*)&lcl_ipv4,sizev4);
	}
	
	else if(version==6){
		ipv6=malloc(40);
		memset(ipv6,0,40);
		fscanf(strUpConf,"%s",ipv6);

		addr_ipv6=(struct sockaddr_in6*)malloc(sizev6);
		setAddrIpv6(addr_ipv6,5001,ipv6);
		
		addr=(struct sockaddr*)addr_ipv6;
		size=sizev6;
		
   	memset(&lcl_ipv6,0,sizev6);
		setAddrIpv6(&lcl_ipv6,5003,"::1");	
			
    sock=creatSocketIpv6();
		bindSock(sock,(struct sockaddr*)&lcl_ipv6,sizev6);
	}
  
	//read server id and fill sync
	srvId=0;
	fscanf(strUpConf,"%d",&srvId);
	sync=(unsigned char)srvId;
	fclose(strUpConf);
	
	//send keepalive(SYNC) message
	if(sendto(sock,&sync,1,0,addr,size)<0){
		perror("sending sync failed");
	}

	//receive ACK
	if(recvfrom(sock,&ack,1,0,NULL,NULL)<0){
		perror("receiving ack failed");
	}
	
	if(ack!=0x00){
		if(sync==0){
			srvId=(int)ack;
			strUpConf=fopen("config","a");
			fprintf(strUpConf,"%d",srvId);
			fclose(strUpConf);
		}
		
		//create and save socket in global variables for updates
		if(version==4){
			addr_ipv4->sin_port=htons(5004);
			glb_addrv4=addr_ipv4;
			rly_srvip=(struct sockaddr*)addr_ipv4;		
			updSock=sock;
			//bindSock(updSock,(struct sockaddr*)glb_addrv4,sizev4);
		}	

		else if(version==6){
			addr_ipv6->sin6_port=htons(5004);
			glb_addrv6=addr_ipv6;
			rly_srvip=(struct sockaddr*)addr_ipv6;	
			updSock=sock;
			//bindSock(updSock,(struct sockaddr*)glb_addrv6,sizev6);
		}
		
		return 0;
	}
	else{
		perror("Relay Server reached maximum limit");
		return -1;
	}
}


int main(){
	char test[]="update";
	
	int x=helloRelay();
	if(x==0){
		if(sendto(updSock,test,6,0,rly_srvip,sizev4)<0){
			perror("send update failed");
		}
	}	
}
