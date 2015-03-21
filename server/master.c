#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define BSIZE1 6
#define BSIZE2 1
#define MAX(x,y) (x>=y) ? x : y

struct sockets{
	int type; //if ipv4 type=4 if ipv6 type=6 if none type=0
	int sock_ipv4;
	int sock_ipv6;
};

int writeFile(char *filename,unsigned int maxSrvId);
int readFile(char *filename);
unsigned char assignId();
void * handleUpdts(void *sockets);
void * addServer(void *sockets);
int free_mem();
int set_ptr_null();

pthread_mutex_t mtx;
struct sockaddr_in *ipv4[20];
struct sockaddr_in6 *ipv6[20];

int main(){

	int socket_ipv4,socket_ipv6,socket_kpalv4,socket_kpalv6;
	struct sockaddr_in addr_ipv4,addr_kpalv4;
	struct sockaddr_in6 addr_ipv6,addr_kpalv6;	
	struct sockets socks_ipv4,socks_ipv6,socks_kpalv;
	pthread_t thread_ipv4,thread_ipv6,thread_addserver;	

	pthread_mutex_init(&mtx, NULL);

	if((socket_ipv4=socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("cannot create ipv4 socket");
		return -1;	
	}
	if((socket_ipv6=socket(AF_INET6,SOCK_DGRAM,0))<0){
		perror("cannot create ipv6 socket");
		return -1;	
	}
	if((socket_kpalv4=socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("cannot create ipv4 keepalive socket");
		return -1;	
	}
	if((socket_kpalv6=socket(AF_INET6,SOCK_DGRAM,0))<0){
		perror("cannot create ipv6 keepalive socket");
		return -1;	
	}

	memset(&addr_ipv4,0,sizeof(addr_ipv4));
	addr_ipv4.sin_family=AF_INET;
	addr_ipv4.sin_port=htons(5004);
	if(inet_pton(AF_INET,"127.0.0.1",&addr_ipv4.sin_addr)<0){
		perror("Ipv4 address assignment failed");
		return -1;
	}
	memset(&addr_ipv6,0,sizeof(addr_ipv6));
	addr_ipv6.sin6_family=AF_INET6;
	addr_ipv6.sin6_port=htons(5004);
	if(inet_pton(AF_INET6,"::1",&addr_ipv6.sin6_addr)<0){
		perror("Ipv6 address assignment failed");
		return -1;
	}
	memset(&addr_kpalv4,0,sizeof(addr_kpalv4));
	addr_kpalv4.sin_family=AF_INET;
	addr_kpalv4.sin_port=htons(5001);
	if(inet_pton(AF_INET,"127.0.0.1",&addr_kpalv4.sin_addr)<0){
		perror("Ipv4 address assignment failed");
		return -1;
	}
	printf("portkkk%hd\n",ntohs(addr_kpalv4.sin_port));
	memset(&addr_kpalv6,0,sizeof(addr_kpalv6));
	addr_kpalv6.sin6_family=AF_INET6;
	addr_kpalv6.sin6_port=htons(5001);
	if(inet_pton(AF_INET6,"::1",&addr_kpalv6.sin6_addr)<0){
		perror("Ipv6 address assignment failed");
		return -1;
	}

	if(bind(socket_ipv4,(struct sockaddr*)&addr_ipv4,sizeof(addr_ipv4))<0){
		perror("bind failed for ipv4");
		return -1;	
	}
	if(bind(socket_ipv6,(struct sockaddr*)&addr_ipv6,sizeof(addr_ipv6))<0){
		perror("bind failed for  ipv6");
		return -1;	
	}
	if(bind(socket_kpalv4,(struct sockaddr*)&addr_kpalv4,sizeof(addr_kpalv4))<0){
		perror("bind failed for kpalive ipv4");
		return -1;	
	}
	if(bind(socket_kpalv6,(struct sockaddr*)&addr_kpalv6,sizeof(addr_kpalv6))<0){
		perror("bind failed for kpalive ipv6");
		return -1;	
	}
	
	set_ptr_null();
	
	socks_ipv4.type=4;
	socks_ipv4.sock_ipv4=socket_ipv4;
	socks_ipv4.sock_ipv6=socket_ipv6;

	socks_ipv6.type=6;
	socks_ipv6.sock_ipv4=socket_ipv4;
	socks_ipv6.sock_ipv6=socket_ipv6;

	socks_kpalv.type=0;
	socks_kpalv.sock_ipv4=socket_kpalv4;
	socks_kpalv.sock_ipv6=socket_kpalv6;
	printf("main sock:%d\n",socks_kpalv.sock_ipv4);
	pthread_create(&thread_ipv4,NULL, handleUpdts, (void *)&socks_ipv4);
	pthread_create(&thread_ipv6,NULL, handleUpdts, (void *)&socks_ipv6);
	pthread_create(&thread_addserver,NULL, addServer, (void *)&socks_kpalv);

  pthread_join(thread_ipv4,NULL);
  pthread_join(thread_ipv6,NULL);
	pthread_join(thread_addserver,NULL);

	pthread_mutex_destroy(&mtx);

	free_mem();
}

void * handleUpdts(void *sockets){
	
	char buff[BSIZE1];
	struct sockets *socks;
	int size,idxv4,idxv6;
	struct sockaddr_in rmtAddr_ipv4;
	struct sockaddr_in6 rmtAddr_ipv6;
	struct sockaddr *rmtAddr;
	int lclsock;
	int sizev4=sizeof(struct sockaddr_in);
	int sizev6=sizeof(struct sockaddr_in6);

	printf("hanl\n");
	if(sockets){
		socks=(struct sockets*)sockets;
		if(socks->type==4){
			lclsock=socks->sock_ipv4;
			size=sizev4;
			memset(&rmtAddr_ipv4,0,sizev4);
			rmtAddr=(struct sockaddr*)&rmtAddr_ipv4;
		}
		else if(socks->type==6){
			lclsock=socks->sock_ipv6;
			size=sizev6;
			memset(&rmtAddr_ipv6,0,sizev6);
			rmtAddr=(struct sockaddr*)&rmtAddr_ipv6;
		}
		for(;;){
			
			memset(&buff,0,BSIZE1);
			
			if(recvfrom(lclsock,buff,BSIZE1,0,rmtAddr,&size)<0){
				perror("receive failed");	
				
			}	
			//ipv4 addresses
			idxv4=0;
			pthread_mutex_lock (&mtx);
			while(ipv4[idxv4]){
				if(sendto(socks->sock_ipv4,buff,BSIZE1,0,(struct sockaddr*)ipv4[idxv4],sizev4)<0){
					perror("send failed");							
				}
				idxv4++;
			}
			pthread_mutex_unlock (&mtx);
	  	//ipv6 addresses
			idxv6=0;
			pthread_mutex_lock(&mtx);
			while(ipv6[idxv6]){
				if(sendto(socks->sock_ipv6,buff,BSIZE1,0,(struct sockaddr*)ipv6[idxv6],sizev6)<0){
					perror("send failed");							
				}
				idxv6++;
			}			
			pthread_mutex_unlock (&mtx);
		}
	}	
	pthread_exit((void*) 0);	
}	

void * addServer(void * sockts){
	
	struct sockaddr_in *addr_ipv4;
	struct sockaddr_in6 *addr_ipv6;
	struct sockets *socks;
	unsigned char buff[BSIZE2];
	char ack;
	int idxv4,idxv6;
	fd_set rset;
	int maxfd;
	int maxSrvId;
	int sizev4=sizeof(struct sockaddr_in);
	int sizev6=sizeof(struct sockaddr_in6);
	
	printf("addServerStarted\n");

  if(sockts){
		idxv4=0;
		idxv6=0;
		socks=(struct sockets*)sockts;
		maxfd=MAX(socks->sock_ipv4,socks->sock_ipv6)+1;
		for(;;){
			FD_ZERO(&rset);
			printf("add sock:%d\n",socks->sock_ipv4);
			FD_SET(socks->sock_ipv4,&rset);
			FD_SET(socks->sock_ipv6,&rset);
			if(select(maxfd,&rset,NULL,NULL,NULL)<0){
				perror("error in select");
				continue;
			}		
			printf("select\n");
			if(FD_ISSET(socks->sock_ipv4,&rset)){
		
				memset(buff,0,BSIZE2);
				addr_ipv4=(struct sockaddr_in*)malloc(sizev4);
				memset(addr_ipv4,0,sizev4);
				if(recvfrom(socks->sock_ipv4,buff,BSIZE2,0,(struct sockaddr*)addr_ipv4,&sizev4)<0){
					perror("receive keepalive failed");					
				}			
			  printf("reved %c\n",buff[0]);
				//check first byte to know if it is from a new server
				if(buff[0]==0)
				{
					printf("in buff\n");
					//assign new id for server and save
					buff[0]=assignId();
				}

				//save remote server address
				if(idxv4<21){
					pthread_mutex_lock(&mtx);
					ipv4[idxv4]=addr_ipv4;
					idxv4++;
					pthread_mutex_unlock(&mtx);
				}				

				//send ack
				printf("port%hu\n",ntohs(addr_ipv4->sin_port));
				sizev4=sizeof(struct sockaddr_in);
				if(idxv4<21){
					if(sendto(socks->sock_ipv4,buff,BSIZE2,0,(struct sockaddr*)addr_ipv4,sizev4)<0){
						perror("send ack failed");
					}
				}
				else{
					//maximum server limit reached
					buff[0]=0x00; 
					if(sendto(socks->sock_ipv4,buff,BSIZE2,0,(struct sockaddr*)addr_ipv4,sizev4)<0){
						perror("send ack failed");
					}
				}
					
			}
		
			if(FD_ISSET(socks->sock_ipv6,&rset)){
				memset(&buff,0,BSIZE1);
				addr_ipv6=(struct sockaddr_in6*)malloc(sizev6);
				memset(addr_ipv6,0,sizev6);
				//receive sync
				if(recvfrom(socks->sock_ipv6,buff,BSIZE2,0,(struct sockaddr*)addr_ipv6,&sizev6)<0){
					perror("receive keepalive failed");	
				}	
				//check first byte to know if it is from a new server
				if(buff[0]==0x00)
				{
					//assign new id for server and save
					buff[0]=assignId();
				}

				//save socket address
				if(idxv6<21){
					pthread_mutex_lock(&mtx);
					ipv6[idxv6]=addr_ipv6;
					idxv6++;
					pthread_mutex_unlock(&mtx);
				}				

				//send ack
				if(idxv6<21){
					if(sendto(socks->sock_ipv6,buff,BSIZE2,0,(struct sockaddr*)addr_ipv6,sizev6)<0){
						perror("send ack failed");
					}
				}
				else{
					//maximum server limit reached
					buff[0]=0x00; 
					if(sendto(socks->sock_ipv6,buff,BSIZE2,0,(struct sockaddr*)addr_ipv6,sizev6)<0){
						perror("send ack failed");
					}
				}
				
			}	
		}
	}
	pthread_exit((void*) 0);	
}

int readFile(char *filename){
	FILE * conf;
	int maxServId;
	maxServId=0;
	conf=fopen(filename,"r");	
	fscanf(conf,"%d",&maxServId);
	fclose(conf);
	return maxServId;
}

int writeFile(char *filename,unsigned int maxSrvId){
	FILE * conf;
	conf=fopen(filename,"w");
	fprintf(conf,"%u",maxSrvId);
	fclose(conf);
	return 1;
}

unsigned char assignId(){
	unsigned int maxSrvId;
	unsigned char buff;
	maxSrvId=readFile("conf");
	maxSrvId++;
	buff=(unsigned char)maxSrvId;
	writeFile("conf",maxSrvId);
	return buff;
}
int set_ptr_null(){
	int i;
	for(i=0;i<21;i++){
		ipv4[i]=NULL;
		ipv6[i]=NULL;
	}
}
int free_mem(){
	int i;
	for(i=0;i<21;i++){
		if(ipv4[i] || ipv6[i]){
			if(ipv4[i])
				free(ipv4[i]);
			if(ipv6[i])
				free(ipv6[i]);
		}
		else
			break;
	}
}
