#ifndef __HTTP_INFO_H__
#define __HTTP_INFO_H__

typedef struct _http_info{
    int is_static;
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];
    char filename[MAXLINE];
    char cgiargs[MAXLINE];
    int connfd;
}http_info;

#endif