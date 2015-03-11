// UDP echo client
// modified from W.R. Stevens example, Fig. 8.7, udpcliserv/udpcli01.c

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>

#define SERV_PORT 7000
#define MAXLINE 80


void dg_cli(FILE *fp, int sockfd,
        const struct sockaddr *pservaddr, socklen_t servlen)
{
    int     n;
    char    sendline[MAXLINE], recvline[MAXLINE + 1];

    // Gets a line from any file, in this case stdin
    while (fgets(sendline, MAXLINE, fp) != NULL) {

    // Send line to socket
        if (sendto(sockfd, sendline, strlen(sendline), 0,
                   pservaddr, servlen) < 0) {
            perror("sendto");
            return;
        }

        // Wait for echo
        // Does is come from the server given above?
        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0,
                      NULL, NULL)) < 0) {
            perror("recvfrom");
            return;
        }

        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return;
        }
    }
}


int main(int argc, char **argv)
{
    int         sockfd, n;
    //struct sockaddr_in6   servaddr;
    struct addrinfo hints, *res, *ressave;

    if (argc != 3) {
        fprintf(stderr, "usage: udpcli <name> <serv/port>\n");
        return -1;
    }

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        fprintf(stderr, "tcp_connect error for %s, %s: %s\n",
                        argv[1], argv[2], gai_strerror(n));
        return -1;
    }
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype,
                                res->ai_protocol);
        if (sockfd < 0)
            continue;       /* ignore this one */

        break;
    } while ((res = res->ai_next) != NULL);

    freeaddrinfo(ressave);

    if (res == NULL) {
        fprintf(stderr, "Could not open socket\n");
        return -1;
    }

    dg_cli(stdin, sockfd, res->ai_addr, res->ai_addrlen);

    return 0;
}