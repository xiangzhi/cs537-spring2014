#include "mfs.h"
#include <stdbool.h>


int send_fd;
struct sockaddr_in saddr;

int udp_Send(char* sendMsg, char* reply);

int MFS_Init(char *hostname, int port){

    send_fd = UDP_Open(0);
    assert(send_fd > -1);

    //printf("hostName:%s port:%d\n", hostname, port);
    int status = UDP_FillSockAddr(&saddr, hostname, port);
    assert(status == 0);

    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    sprintf(message, "I");
    udp_Send(message, buffer);
    return 0;
}

int MFS_Lookup(int pinum, char *name){

    //make sure the input is valid;
    if(pinum < 0 || strlen(name) >= 60){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "L:%d:%s", pinum, name);
    udp_Send(message, reply);
    return atoi(reply);
}

int MFS_Stat(int inum, MFS_Stat_t *m){

    //make sure the input is valid;
    if(inum < 0 || m == NULL){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "S:%d", inum);
    udp_Send(message, reply);
    MFS_Stat_t r_stat = *((MFS_Stat_t *)reply);
    //state is incoorect
    if(r_stat.type > 1 || r_stat.type < 0){
        return -1;
    }
    m->type = r_stat.type;
    m->size = r_stat.size;
    return 0;
}

int MFS_Write(int inum, char *buffer, int block){

    //make sure the input is valid;
    if(inum < 0 || block < 0 || block > 14){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "W:%d:%d", inum, block);
    udp_Send(message, reply);
    //make sure the reply is 0;//success
    if(atoi(reply) != 0){
        return -1;
    }
    udp_Send(buffer, reply);
    //make sure the reply is 0;//success
    if(atoi(reply) != 0){
        return -1;
    }
    return 0; 
}

int MFS_Read(int inum, char *buffer, int block){

    //make sure the input is valid;
    if(inum < 0 || block < 0 || block > 14){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "R:%d:%d:p1", inum, block);
    udp_Send(message, reply);
    //make sure the reply is 0;//success
    if(atoi(reply) != 0){
        return -1;
    }
    snprintf(message, BUFFER_SIZE, "R:%d:%d:p2", inum, block);
    udp_Send(message, buffer);
    return 0;
}

int MFS_Creat(int pinum, int type, char *name){

    //make sure the input is valid;
    if(pinum < 0 || strlen(name) >= 60 || type > 1 || type < 0){
        printf("invalid input");
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "C:%d:%d:%s", pinum, type, name);
    udp_Send(message, reply);
    return atoi(reply);
}

int MFS_Unlink(int pinum, char *name){

    //make sure the input is valid;
    if(pinum < 0 || strlen(name) >= 60){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "U:%d:%s", pinum, name);
    udp_Send(message, reply);
    return atoi(reply);
}

int MFS_Shutdown(){
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "E:");
    udp_Send(message, reply);
    return atoi(reply);
}

int udp_Send(char* sendMsg, char* reply){

    //initalize the varaibles
    fd_set readfd;
    fd_set emptyfd;
    FD_ZERO(&emptyfd);
    FD_ZERO(&readfd);
    FD_SET(send_fd, &readfd);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    while(true){
        //printf("sending\n");
        int status = UDP_Write(send_fd, &saddr, sendMsg, BUFFER_SIZE);
        if(status <= 0){
            printf("error in write\n");
            continue;
        }
        //printf("waiting\n");
        status = select( send_fd +1, &readfd, &emptyfd, &emptyfd, &timeout);
        if(status == 1){
            struct sockaddr_in raddr;
            status = UDP_Read(send_fd, &raddr, reply, BUFFER_SIZE);
            //printf("CLIENT:: read %d bytes (message: '%s')\n", status, reply);
            break;
        }
    }
    return 0;
}