/* CS537 - Spring 2014 - Program 5
 * CREATED BY:  
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * Roy Fang (fang@cs.wisc.edu)
 */

#include "mfs.h"
#include <stdbool.h>

//store the sending socket
int send_fd;
//information to where to send
struct sockaddr_in saddr;
//flag to say whether server is online
bool server_stat = false;
//prototypes
int udp_Send(char* sendMsg, char* reply, bool wait);

//implementations

int MFS_Init(char *hostname, int port){

    //open file and make sure its connected
    send_fd = UDP_Open(0);
    assert(send_fd > -1);

    int status = UDP_FillSockAddr(&saddr, hostname, port);
    assert(status == 0);

    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    //send a heartbeat to making sure the connection is valid
    sprintf(message, "I");
    udp_Send(message, buffer, true);
    server_stat = true;
    return 0;
}

int MFS_Lookup(int pinum, char *name){

    //make sure the input is valid;
    if(pinum < 0 || strlen(name) >= 60){
        return -1;
    }

    //make sure server is runnig
    if(!server_stat){
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

    //make sure server is runnig
    if(!server_stat){
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

    //make sure server is runnig
    if(!server_stat){
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
        //check whether got response or timeout
        status = udp_Send(buffer, reply, false);
    }
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

    //make sure server is runnig
    if(!server_stat){
        return -1;
    } 

    int status = -1;
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "R:1:%d:%d", inum, block);
    while(status == -1){
        //try sending the response
        udp_Send(message, reply, true);
        //make sure the reply is 0;//success
        if(atoi(reply) != 0){
            return -1;
        }
        snprintf(message, BUFFER_SIZE, "R:2");
        //check whether got response or timeout
        status = udp_Send(message, buffer, false);
    }
    return 0;
}

int MFS_Creat(int pinum, int type, char *name){

    //make sure the input is valid;
    if(pinum < 0 || strlen(name) >= 60 || type > 1 || type < 0){
        printf("invalid input");
        return -1;
    }

    //make sure server is runnig
    if(!server_stat){
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

    //make sure server is runnig
    if(!server_stat){
        return -1;
    }

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "U:%d:%s", pinum, name);
    udp_Send(message, reply, true);
    return atoi(reply);
}

int MFS_Shutdown(){

    //make sure server is runnig
    if(!server_stat){
        return -1;
    } 

    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "E:");
    udp_Send(message, reply, false);
    return 0;
}

/**
 * udp_Send
 * wrapper function for sending UDP
 * waits for the return reply, retry untill got response.
 * - sendMsg: 4096 byte charater to send
 * - reply: pointer to store response
 * - wait: whether to keep retrying mesage
 * return: whether it was successful
 */
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
        //wait for the response for 5 seconds
        int rs = select( send_fd +1, &readfd, &emptyfd, &emptyfd, &timeout);    
        if(rs > 0 && FD_ISSET(send_fd, &readfd)){
            struct sockaddr_in raddr;
            status = UDP_Read(send_fd, &raddr, reply, BUFFER_SIZE);

            //making sure UDP_Read got the whole message.
            if(status != BUFFER_SIZE){
                if(!wait){
                    return -1;
                }
                else{
                    continue;
                }
            }
            break;
        }
        else{
            //timeout, see whether to return
            if(!wait){
                return -1;
            }
        }
    }
    return 0;
}