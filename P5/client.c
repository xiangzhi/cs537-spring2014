#include <stdio.h>
#include "mfs.h"



int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);


int
main(int argc, char *argv[])
{

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    /*
    int sd = UDP_Open(0);
    assert(sd > -1);

    struct sockaddr_in saddr;
    int rc = UDP_FillSockAddr(&saddr, "localhost", 7997);
    assert(rc == 0);

    printf("CLIENT:: about to send message (%d)\n", rc);
    ;
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
    //int rtn = MFS_Creat(0, 0, "firDir");
    //printf("return from creat:%d\n", rtn);
    int rtn = MFS_Creat(0, 1, "firstFILE");
    printf("return from creat:%d\n", rtn);
    int code = MFS_Lookup(0, "firstFILE");
    printf("return froom lookup %d\n", code);    
    strncpy(message, "START BLOCK 1 END BLOCK 1", 4096);
    printf("toSend:%s\n", message);
    rtn = MFS_Write(code ,message, 0);
    printf("return from write %d\n", rtn);
    rtn = MFS_Read(code , reply, 0);
    printf("return from read %d result:%s\n", rtn, reply);
    /*
    rtn = MFS_Lookup(0, "firDir");
    printf("return froom lookup %d\n", rtn);
    rtn = MFS_Lookup(1, ".");
    printf("return froom lookup %d\n", rtn);
    rtn = MFS_Lookup(1, "..");
    printf("return froom lookup %d\n", rtn);
    rtn = MFS_Creat(1, 1, "firstFile");
    printf("return froom Create 2 %d\n", rtn);   
    rtn = MFS_Lookup(1, "firstFile");
    printf("return froom lookup %d\n", rtn);
    strncpy(message, "Hello World", 4096);
    rtn = MFS_Write(2,message, 1);
    printf("return from write %d\n", rtn);
    rtn = MFS_Read(2, reply, 1);
    printf("return from read %d result:%s\n", rtn, reply);

    MFS_Stat_t stat_t;
    rtn = MFS_Stat(2 , &stat_t);
    printf("return from stat: size:%d, type:%d\n", stat_t.size, stat_t.type);

    rtn = MFS_Read(1, reply, 0);
    MFS_DirEnt_t stat = *((MFS_DirEnt_t*)reply);
    printf("return from read %d name:%s, inum:%d\n", rtn, stat.name, stat.inum);
    rtn = MFS_Unlink(1, "firstFile");
    printf("return froom unlink %d\n", rtn);
    rtn = MFS_Lookup(1, "firstFile");
    printf("return froom lookup after unlink %d\n", rtn);
    rtn = MFS_Stat(1 , &stat_t);
    printf("return from stat: size:%d, type:%d\n", stat_t.size, stat_t.type);
    */


    /*
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
    printf("return from unlink:%d\n", rtn);*/
    printf("shut:%d", MFS_Shutdown());
    return 0;
}


