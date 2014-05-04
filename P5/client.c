#include <stdio.h>
#include "mfs.h"



int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);


int
main(int argc, char *argv[])
{
    /*
    int sd = UDP_Open(0);
    assert(sd > -1);

    struct sockaddr_in saddr;
    int rc = UDP_FillSockAddr(&saddr, "localhost", 7997);
    assert(rc == 0);

    printf("CLIENT:: about to send message (%d)\n", rc);
    char message[BUFFER_SIZE];
    sprintf(message, "hello world");
    rc = UDP_Write(sd, &saddr, message, BUFFER_SIZE);
    // printf("CLIENT:: sent message (%d)\n", rc);
    if (rc > 0) {
	struct sockaddr_in raddr;
	int rc = UDP_Read(sd, &raddr, buffer, BUFFER_SIZE);
	printf("CLIENT:: read %d bytes (message: '%s')\n", rc, buffer);
    }
    */
    MFS_Init("localhost", 7997);
    int rtn = MFS_Lookup(99, "test.rr");
    printf("return %d\n", rtn);
    MFS_Stat_t stat;
    MFS_Stat(99, &stat);
    printf("size:%d, type:%d\n", stat.size, stat.type);
    char buffer[4096];
    strncpy(buffer, "hello world", 4096);
    rtn = MFS_Write(99, buffer, 1);
    printf("return from write:%d\n", rtn);
    rtn = MFS_Read(99, buffer, 1);
    printf("return from read:%d content:%s\n", rtn, buffer);
    rtn = MFS_Creat(99, 1, "yolo");
    printf("return from creat:%d\n", rtn);
    rtn = MFS_Unlink(79,"yooooso");
    printf("return from unlink:%d\n", rtn);
    MFS_Shutdown();
    return 0;
}


