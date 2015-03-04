#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>

#include "data_structure.h"

#include "proto_types.h"

void * ProcessThreadStart()
{

     printf("ProcessThreadStart created\n");
 
return NULL;
}

void * SyncThreadStart()
{

     printf("SyncThreadStart created\n");
return NULL;
}


