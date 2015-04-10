#include "sys/select.h"
#include "common.c"
void setmsg(char *msg,char*srvid,char*msgtype,char *result){
	if(result){
		strcat(result,msgtype);
		strcat(result,DELIMITER);
		strcat(result,srvid);
		strcat(result,DELIMITER);
		strcat(result,msg);
	}
}
void parseSrvId(char *msg,char *srvId){
	char *tempSrvId;
  if(msg && srvId){
		memset(srvId,0,6);
		if((tempSrvId=parseTocken(msg,2,DELIMITER))!=NULL){
			if(strlen(tempSrvId)!=5)
				printf("Error: Master server response is incorrect");
			else
				strcpy(srvId,tempSrvId);
			free(tempSrvId);
		}
  	else
			printf("Error: Master server response is incorrect");
	}
}
int main(){
	char msg[100];
	char srvId4[6],srvId6[6];
	uint16_t port=5004;
	uint16_t rm_port=5003;
	skaddr_in addr4,rm_addr4;
	skaddr_in6 addr6,rm_addr6;
	int sock4,sock6;
	int size4=sizeof(skaddr_in);
	int size6=sizeof(skaddr_in6);
	fd_set fdset;
	//create sockets
	printf("Create socket.\n");
	sock4=creatUdpSocketIpv4();
	sock6=creatUdpSocketIpv6();

	//setup addresses
	printf("Setting up local addresses.\n");
	setAddrIpv4(&addr4,port,"127.0.0.1");
	setAddrIpv6(&addr6,port,"::1");

	//bind sockets
	printf("Binding to socket.\n");
	bindSock(sock4,(skaddr*)&addr4,size4);
	bindSock(sock6,(skaddr*)&addr6,size6);

	//setup remote addresses
	printf("Setting up remote addresses.\n");
	setAddrIpv4(&rm_addr4,rm_port,"127.0.0.1");
	setAddrIpv6(&rm_addr6,rm_port,"::1");

//TEST2.1 STARTING:
	printf("\nTEST 2.1 (JOIN) AS NEW SERVER STARTING\n");

	//send ipv4 JOIN$00000 message to master server
	printf("IPV4: sending ipv4 JOIN$00000 message.\n");
	sendMsg("JOIN$00000",sock4,(skaddr *)&rm_addr4,size4);
	
	//waiting for JOIN response
	printf("IPV4: waiting for JOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock4,msg,NULL,size4);
	printf("IPV4: Join response received : %s\n",msg);

	//Parse server id from the message
	printf("IPV4: Parse server id from the message\n");
  parseSrvId(msg,srvId4);
	
	//send ipv6 JOIN$00000 message to master server
	printf("IPV6: sending ipv6 JOIN$00000 message.\n");
	sendMsg("JOIN$00000",sock6,(skaddr *)&rm_addr6,size6);
	
	//waiting for JOIN response
	printf("IPV6: waiting for JOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock6,msg,NULL,size6);
	printf("IPV6: Join response received : %s\n",msg);

	//Parse server id from the message
	printf("IPV6: Parse server id from the message\n");
  parseSrvId(msg,srvId6);

	printf("TEST 2.1 FINISHED\n");
//TEST2.1 FINISHED

//TEST3 STARTING
	printf("\nTEST 3 (DISJOIN) STARTING\n");
	
	//send ipv4 DISJOIN$00001 message to master server
	memset(msg,0,100);
	setmsg("",srvId4,"DISJOIN",msg);
	printf("IPV4: sending ipv4 DISJOIN message:%s\n",msg);
	sendMsg(msg,sock4,(skaddr *)&rm_addr4,size4);
	
	//waiting for DISJOIN response
	printf("IPV4: waiting for DISJOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock4,msg,NULL,size4);
	printf("IPV4: DISJoin response received : %s\n",msg);

	//send ipv6 DISJOIN$00002 message to master server
	memset(msg,0,100);
	setmsg("",srvId6,"DISJOIN",msg);
	printf("IPV6: sending ipv6 DISJOIN message: %s\n",msg);
	sendMsg(msg,sock6,(skaddr *)&rm_addr6,size6);
	
	//waiting for JOIN response
	printf("IPV6: waiting for DISJOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock6,msg,NULL,size6);
	printf("IPV6: DISJoin response received : %s\n",msg);

	printf("TEST 3 FINISHED\n");
//TEST3 FINISHED

//TEST2.2 STARTING:
	printf("\nTEST 2.2 (JOIN) AS OLD SERVER STARTING\n");

	//send ipv4 JOIN message to master server
	memset(msg,0,100);
	setmsg("",srvId4,"JOIN",msg);
	printf("IPV4: sending ipv4 JOIN message: %s\n",msg);
	sendMsg(msg,sock4,(skaddr *)&rm_addr4,size4);
	
	//waiting for JOIN response
	printf("IPV4: waiting for JOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock4,msg,NULL,size4);
	printf("IPV4: Join response received : %s\n",msg);

	/*Parse server id from the message
	printf("IPV4: Parse server id from the message\n");
  parseSrvId(msg,srvId4);*/
	
	//send ipv6 JOIN message to master server
	memset(msg,0,100);
	setmsg("",srvId6,"JOIN",msg);
	printf("IPV6: sending ipv6 JOIN message: %s\n",msg);
	sendMsg(msg,sock6,(skaddr *)&rm_addr6,size6);
	
	//waiting for JOIN response
	printf("IPV6: waiting for JOIN response.\n");
	memset(msg,0,100);
	rcvMsg(sock6,msg,NULL,size6);
	printf("IPV6: Join response received : %s\n",msg);

	/*Parse server id from the message
	printf("IPV6: Parse server id from the message\n");
  parseSrvId(msg,srvId6);*/

	printf("TEST 2.2 FINISHED\n");
//TEST2.2 FINISHED

//TEST4 STARTING
	printf("\nTEST 4 (UPDATE) STARTING\n");
	
	//send ipv4 UPDATE message
  memset(msg,0,100);
	setmsg("MSG$FROM$IPV4SEVER",srvId4,"UPDATE",msg);
	printf("IPV4: sending ipv4 UPDATE message:%s\n",msg);
	sendMsg(msg,sock4,(skaddr *)&rm_addr4,size4);

	//send ipv6 UPDATE message
	memset(msg,0,100);
	setmsg("MSG$FROM$IPV6SEVER",srvId6,"UPDATE",msg);
	printf("IPV6: sending ipv6 UPDATE message: %s\n",msg);
	sendMsg(msg,sock6,(skaddr *)&rm_addr6,size6);

	//IPV4 waiting for UPDATE response
	printf("IPV4: waiting for UPDATE.\n");
	memset(msg,0,100);
	rcvMsg(sock4,msg,NULL,size4);
	printf("IPV4: UPDATE received : %s\n",msg);
	
	//IPV6 waiting for UPDATE response
	printf("IPV6: waiting for UPDATE.\n");
	memset(msg,0,100);
	rcvMsg(sock6,msg,NULL,size6);
	printf("IPV6: UPDATE received : %s\n",msg);

	printf("TEST 4 FINISHED\n");
//TEST4 FINISHED

	
}
