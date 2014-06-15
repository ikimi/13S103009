/*
 * Copyright 2014 Jiajun Li
 * All rights reserved
 *
 * 获取HTTP报文的request-line
 * 包括method.uri,version三项
 */

#include <stdio.h>

void read_requestline(char *data, char *method, char *uri, char *version)
{
    sscanf(data, "%s %s %s", method, uri, version);
}
