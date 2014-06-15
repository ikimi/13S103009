#ifndef CLIENTERROR_H_INCLUDED
#define CLIENTERROR_H_INCLUDED

void clienterror(int connfd, char *version, char *uri,
        char *errno, char *shortmsg, char *longmsg);

#endif
