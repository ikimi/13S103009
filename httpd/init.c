/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 初始化服务器
 * 在port端口设置监听套接字,监听来自客户端的连接请求
 */

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

int init(unsigned port, struct sockaddr_in *servaddr)
{
    int listenfd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket error\n");
        return -1;
    }

    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons(port);
    servaddr->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)servaddr, sizeof(struct sockaddr)) < 0)
    {
        fprintf(stderr, "bind error\n");
        return -1;
    }

    if (listen(listenfd, 100) < 0)
    {
        fprintf(stderr, "listen error\n");
        return -1;
    }

    return listenfd;
}
