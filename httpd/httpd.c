/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 小型Web服务器,目前只支持GET方法
 * 可以动态设定端口号,默认8080端口
 * 采用多进程的方式响应用户请求
 * 支持对html/text/jpg/jpeg格式文档的请求
 * 支持简单的错误发现机制, 包括404.403.501
 * 支持静态响应, 也支持动态响应(以cgi的方式)
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include "./include/init.h"
#include "./include/service.h"

void port_error();
void sig_chld(int signo);

int main(int argc, char **argv)
{
    unsigned port;
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t pid;

    // 设置监听端口号
    if (argc == 1)
    {
        if ((listenfd = init(8080, &servaddr)) < 0)
        {
            fprintf(stderr, "分配端口号失败!\n");
            exit(-1);
        }
    }
    else if (argc == 2)
    {
        port = atoi(argv[1]);
        if (port < 5000)
        {
            port_error();
            exit(-1);
        }
        if ((listenfd = init(port, &servaddr)) < 0)
        {
            fprintf(stderr, "分配端口失败!\n");
            exit(-1);
        }
    }
    else
    {
        port_error();
        exit(-1);
    }

    printf("******************************\n");
    printf("Welcome to kimi's server!\n");
    printf("*Port: %d\n", port);
    time_t t;
    time(&t);
    printf("*Date: %s", ctime(&t));
    printf("******************************\n");
    // 捕捉子进程结束信号，进入sig_chld信号处理程序
    signal(SIGCHLD, sig_chld);

    // 监听套接字,接收来自client的请求,每一次请求fork出一个进程
    for (;;)
    {
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
            {
                fprintf(stderr, "accept error\n");
                exit (0);
            }
        }
        if ((pid = fork()) == 0)
        {
            close(listenfd);
            service(connfd);
            exit(0);
        }
        close(connfd);
    }

}

void port_error()
{
    fprintf(stderr, "错误类型：端口号分配失败\n");
    fprintf(stderr, "1. 无参数端口号默认8080\n");
    fprintf(stderr, "2. 一个参数，端口号范围(>=5000)");

    return ;
}

// 防止产生过多的僵尸进程,每当捕获到子进程结束信号时,都
// 使用wait函数,获得其子进程返回信息，释放资源
void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0);

    return ;
}
