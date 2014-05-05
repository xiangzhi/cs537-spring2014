#include "mfs.h"
#include <stdbool.h>


int send_fd;
struct sockaddr_in saddr;

int udp_Send(char* sendMsg, char* reply, bool wait);

int MFS_Init(char *hostname, int port){

    send_fd = UDP_Open(0);
    assert(send_fd > -1);

    //printf("hostName:%s port:%d\n", hostname, port);
    int status = UDP_FillSockAddr(&saddr, hostname, port);
    assert(status == 0);

    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    sprintf(message, "I");
    udp_Send(message, buffer, true);
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
    udp_Send(message, reply, true);
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
    udp_Send(message, reply, true);
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
    if(inum < 0 || block < 0 || block >= 14){
        return -1;
    }
    int status = -1;
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "W:%d:%d", inum, block);
    while(status == -1){
        udp_Send(message, reply, true);
        //make sure the reply is 0;//success
        if(atoi(reply) != 0){
            return -1;
        }
        status = udp_Send(buffer, reply, false);
    }
    /*
    char*  str = "in write\n";
    write(1, str, strlen(str));
    write(1,buffer,strlen(buffer) + 1);
    write(STDOUT_FILENO,"\n",1);
    */

    //make sure the reply is 0;//success
    if(atoi(reply) != 0){
        return -1;
    }
    return 0; 
}

int MFS_Read(int inum, char *buffer, int block){

    //make sure the input is valid;
    if(inum < 0 || block < 0 || block >= 14){
        return -1;
    }
    int status = -1;
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "R:1:%d:%d", inum, block);
    while(status == -1){
        udp_Send(message, reply, true);
        //make sure the reply is 0;//success
        if(atoi(reply) != 0){
            return -1;
        }
        snprintf(message, BUFFER_SIZE, "R:2");
        status = udp_Send(message, buffer, false);
    }
    /*
    char*  str = "in read\n";
    write(1, str, strlen(str));
    write(1,buffer,strlen(buffer) + 1);
    write(STDOUT_FILENO,"\n",1);
    */
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
    udp_Send(message, reply, true);
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
    udp_Send(message, reply, true);
    return atoi(reply);
}

int MFS_Shutdown(){
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "E:");
    udp_Send(message, reply, false);
    return 0;
}

int udp_Send(char* sendMsg, char* reply, bool wait){

    while(true){
        //initalize the varaibles
        fd_set readfd;
        fd_set emptyfd;
        FD_ZERO(&emptyfd);
        FD_ZERO(&readfd);
        FD_SET(send_fd, &readfd);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        //char str[5000];
        //sprintf(str, "sending, msg:%s\n", sendMsg);
        //write(1,str,strlen(str));

        //send the message
        int status = UDP_Write(send_fd, &saddr, sendMsg, BUFFER_SIZE);
        if(status <= 0 || status != BUFFER_SIZE){
            char* str = "error in write\n";
            write(1,str,strlen(str));
            continue;
        }

        //sprintf(str,"waiting\n");
        //write(1,str,strlen(str));

        int rs = select( send_fd +1, &readfd, &emptyfd, &emptyfd, &timeout);    
        if(rs > 0 && FD_ISSET(send_fd, &readfd)){
            //sprintf(str,"receive\n");
            //write(1,str,strlen(str));
            struct sockaddr_in raddr;
            status = UDP_Read(send_fd, &raddr, reply, BUFFER_SIZE);

            if(status != BUFFER_SIZE){
                if(!wait){
                    return -1;
                }
                else{
                    continue;
                }
            }
            //printf("CLIENT:: read %d bytes (message: '%s')\n", status, reply);
            break;
        }
        else{
            if(!wait){
                return -1;
            }
        }
    }
    return 0;
}