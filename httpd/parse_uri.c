/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 解析request的uri
 * 判断请求文档的类型(静态 or 动态)
 * 针对每种情况，产生相应的文件名，对于cgi程序，还依赖于cgi参数
 */

#include <stdio.h>
#include <string.h>

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    // 判断client请求的资源是static的还是dynamic的
    if (!strstr(uri, "cgi-bin"))
    {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);

        // 如果请求的是一个目录,则默认访问该目录下的index.html文档
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "index.html");

        return 1;
    }
    else
    {
        char *ptr;

        if (ptr = index(uri, '?'))
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);

        return 0;
    }
}
