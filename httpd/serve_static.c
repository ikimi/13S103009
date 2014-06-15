/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 为客户端响应静态文档
 * 目前只支持html/text/gif/jpeg格式的文档
 * 对于请求文档的大小没有限制
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

void get_filetype(const char *filename, char *filetype);

void serve_static(int connfd, char *version, char *filename, int filesize)
{
    int fd, n;
    char filetype[20], buf[1000];

    // 发送响应的request-line 和 header
    get_filetype(filename, filetype);
    sprintf(buf, "%s 200 OK\r\n", version);
    sprintf(buf, "%sServer: Jiajun Li's Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    write(connfd, buf, strlen(buf));

    // 发送文档正文
    fd = open(filename, O_RDONLY);
    while ((n = read(fd, buf, 1000)) > 0)
    {
      //  printf("%d\n", n);
        write(connfd, buf, n);
    }

    return ;
}

// 获取文档类型(目前仅仅支持html/纯文本/图片(.gif/.ipeg)格式)
void get_filetype(const char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

