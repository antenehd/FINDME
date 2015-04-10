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

FILE * fpLog;
pthread_mutex_t mtx;
pthread_t threadIpv4;
pthread_t threadIpv6;
uint16_t starSrvId;
int socket_ipv4,socket_ipv6; /*Udp sockets for ipv4 and ipv6 server communication*/
uint16_t maxNmSrvs;


int main(int argc,char *argv[]){
	struct sigaction sig;
	skaddr_in addr_ipv4;
	skaddr_in6 addr_ipv6;
	char ipv4[SIZE_IPV4];
	char ipv6[SIZE_IPV6];
	uint16_t port=0;
	int argParsed=0;
	time_t currTime;
  
	/*daemonize the program*/
	daemon(1,0);

	/*open log file for writing*/
	if((fpLog=fopen(LOG_FILE,"a+"))==NULL)
		LOG("Opening logfile failed. Error:%s\n",strerror(errno));

	currTime = time(NULL);
	LOG("\n%ld : Master server starting...\n",currTime);	
	fflush(fpLog);

	threadIpv4=0;
	threadIpv6=0;
	memset(ipv4,0,SIZE_IPV4);
	memset(ipv6,0,SIZE_IPV6);
	maxNmSrvs=0;

	/*try to read ip addresses and port numbers from terminal*/
	if(argc>2){
		if(parseCmdArg(argc,argv,ipv4,ipv6,&port,&maxNmSrvs)==-1){
			LOG("	Invalid command line arguments.	Exiting...\n");
			return -1;	
		}	
		argParsed=1;
	}

	/*try to read ip addresses and port numbers from configuration file*/ 
	if(argParsed==0){
		if(readAddr(ipv4,ipv6,&port,&maxNmSrvs)==-1){
			LOG("	Reading \"address.conf\" file failed. Exiting...\n");
			printf("\"address.conf\" file is not properly configured\n");
			return -1;
		}
	}
	
	/*setup to handle the specified signals*/
	sigAction(SIGHUP,&sig,signalHandler);
	sigAction(SIGPIPE,&sig,signalHandler);
	sigAction(SIGTERM,&sig,signalHandler);
	/*sigAction(SIGINT,&sig,signalHandler);
	sigAction(SIGQUIT,&sig,signalHandler);
	*/


	/*initialize mutex*/
	pthread_mutex_init(&mtx, NULL);
	
	/*read the Starting server id that will be assigned by this relay server*/
	if(readStarSrvId()==-1){
		printf("Starting server id should found in \"servid\" file in the form: serverId=<starsrvid>\n");
		LOG("	Reading starting server id from \"servid.conf\" failed. Exiting...\n");
		return -1;
	}

	/*start a thread to handle ipv4 messages*/
  if(strlen(ipv4)>0){
		
		socket_ipv4=creatUdpSocketIpv4();
		
		if(setAddrIpv4(&addr_ipv4,port,ipv4)==-1){
			LOG("	Setting up ipv4 address failed. Exiting...\n");
			return -1;
		}
		
		if(bindSock(socket_ipv4,(skaddr*)&addr_ipv4,sizeof(addr_ipv4))==-1){
			LOG("	Bind failed for ipv4 address. Exiting...\n");
			return -1;
  	}			
	}

	/*start thread to handle ipv6 messages*/
	if(strlen(ipv6)>0){

		socket_ipv6=creatUdpSocketIpv6();

		if(setAddrIpv6(&addr_ipv6,port,ipv6)==-1){
			LOG("	Setting up ipv6 address failed. Exiting...\n");
			return -1;
		}
		if(bindSock(socket_ipv6,(skaddr*)&addr_ipv6,sizeof(addr_ipv6))==-1){
			LOG(" Bind failed for ipv6 address. Exiting...\n");
			return -1;
		}		
	}

	/*start ipv4 and ipv6 messages handling threads*/
	if(strlen(ipv4)>0)
		pthread_create(&threadIpv4,NULL, ipv4Msgs, NULL);	
	if(strlen(ipv6)>0)
		pthread_create(&threadIpv6,NULL, ipv6Msgs, NULL);

	/*join terminated threads*/
	if(threadIpv4!=0)
		pthread_join(threadIpv4,NULL);
	if(threadIpv6!=0)
  	pthread_join(threadIpv6,NULL);

	pthread_mutex_destroy(&mtx);

	currTime = time(NULL);
	LOG("%ld : Exiting...\n",currTime);
	return 0;
} 
