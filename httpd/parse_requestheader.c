/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 为了支持多宿主主机,需要解析HTTP报文的header
 * 目前只支持Host选项
 */

#include <stdio.h>
#include <string.h>

void get_requesthost(const char *data, char *host)
{
    int base, len;
    char key[100], value[100];

    base = 0, len = strlen(data);
    while (base < len)
    {
        sscanf(data + base, "%s %s", key, value);
        if (strcmp(key, "Host:") == 0)
        {
            strcpy(host, value);
            return ;
        }
        base += strlen(host) + 2;
    }
    host = NULL;
}
