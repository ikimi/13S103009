/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 响应客户端请求
 * 解析request-line,header
 * 判断文档是否存在(产生404错误)
 * 判断文档的类型(静态 or 动态)
 * 判断文档的权限(产生403错误)
 * 对正确的request相应reponse
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "./include/read_requestline.h"
#include "./include/clienterror.h"
#include "./include/parse_requestheader.h"
#include "./include/serve_static.h"

#define MAXSIZE 1000

char data[MAXSIZE];
char method[10], uri[100], version[10], host[100];
char filename[100], cgiargs[100];

void service(int connfd)
{
    int n;
    struct stat sbuf;

    // 返回client发送来的数据, 存储在data中
    if ((n = recv(connfd, data, MAXSIZE, 0)) > 0)
    {
        // write(STDOUT_FILENO, data, n);
        // 获取 method, uri, version
        read_requestline(data, method, uri, version);

        /*
        printf("method: %s\n", method);
        printf("uri: %s\n", uri);
        printf("version: %s\n", version);
        */
        // 目前只支持GET方法, 其余方法（包括PUT, POST等）随后更新
        if (strcmp(method, "GET") == 0)
        {
            int dist, is_static;

           // clienterror(connfd, version, uri, "200", "OK", "get the resource success");
           // 支持多宿主主机, 对Host选项j进行处理
           // 对于其他选项我们此时暂时忽略处理
           dist = index(data, '\r') - data + 2;
           get_requesthost(data + dist, host);
           // printf("%s\n", host);

           // 解析uri,分析请求对象是static的,还是dynamic的
           is_static = parse_uri(uri, filename, cgiargs);
           printf("%s\n", filename);

           // 获取文档信息, 文档不存在, 返回给客户端404错误信息
           if (stat(filename, &sbuf) < 0)
           {
               clienterror(connfd, version, uri, "404"
                       , "Not Found", "Server couldn't find this file");
               return ;
           }

           if (is_static)
           {
               if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))
               {
                   clienterror(connfd, version, uri, "403"
                           , "Forbidden", "Server could't read the file");
                   return ;
               }

               printf("static\nfilename:%s\n", filename);
               serve_static(connfd, version, filename, sbuf.st_size);
           }
           else
           {
               if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))
               {
                   clienterror(connfd, version, uri, "403"
                           , "Frobidden", "Server couldn't run the CGI program");
                   return ;
               }

               printf("dynamic\nfilename:%s, args:%s\n", filename, cgiargs);
               serve_dynamic(connfd, version, filename, cgiargs);
           }
        }
        else
        {
            printf("501 Not Implemented\n");
            clienterror(connfd, version, uri, "501", "Not Implemented", "Our Server does not implement this method");
        }
    }

    if (n < 0)
    {
        fprintf(stderr, "read error\n");
        exit(-1);
    }
}
