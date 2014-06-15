/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 执行cgi程序，产生动态结果
 * 每一次执行cgi程序，会产生子进程
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void serve_dynamic(int connfd, char *version, char *filename, char *cgiargs)
{
    char buf[1000];
    char **environ;
    char *emptylist[] = {NULL};

    // 输出HTTP响应头部
    sprintf(buf, "%s 200 OK\r\n", version);
    sprintf(buf, "%sServer: Jiajun Li's server\r\n\r\n", buf);
    write(connfd, buf, strlen(buf));

    // 产生动态数据
    if (fork() == 0)
    {
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(connfd, STDOUT_FILENO);
        execve(filename, emptylist, environ);
    }
    wait(0);

    return ;
}
