#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in cliaddr;
    int sockfd;

    if (argc != 2)
    {
        fprintf(stderr, "usage: <ip address>\n");
        exit(-1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&cliaddr, sizeof(struct sockaddr_in));

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &cliaddr.sin_addr);

    connect(sockfd, (struct sockaddr *) &cliaddr, sizeof(struct sockaddr));

    exit(0);
}
