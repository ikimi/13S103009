/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 向客户端返回出错信息
 * 错误信息类型右参数得到
 */

#include <stdio.h>
#include <string.h>

#define MAXSIZE 1000
#define MAXBUF 1000

void clienterror(int connfd, char *version, char *uri
        , char *errno, char *shortmsg, char *longmsg)
{
    char header[MAXSIZE], body[MAXBUF];

    // generate the body
    sprintf(body, "<html><title>Error</title>");
    sprintf(body, "%s<body bgcolor""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errno, shortmsg);
    sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, uri);
    sprintf(body, "%s<hr><em> Copyright by Jiajun Li </em></hr\r\n", body);
    sprintf(body, "%s</body></html>\r\n", body);

    // response line
    sprintf(header, "%s %s %s\r\n", version, errno, shortmsg);
    write(connfd, header, strlen(header));

    // response header
    // Content-type
    sprintf(header, "Content-type: text/html\r\n");
    write(connfd, header, strlen(header));

    // Content-length
    sprintf(header, "Content-length: %d\r\n\r\n", (int)strlen(body));
    write(connfd, header, strlen(header));

    write(connfd, body, strlen(body));
}
