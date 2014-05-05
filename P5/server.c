#include <stdio.h>
#include "udp.h"
#include <stdbool.h>
//connect with the onsite-fs library
#include "fs.h"

#define BUFFER_SIZE (4096)

int processConnection(char input[], char* output, int sd,struct sockaddr_in s);
int udp_wait(char* reply, int sd, struct sockaddr_in s);

int
main(int argc, char *argv[])
{

    // Starting mysh program with incorrect number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: server [portnum] [file-system-image]\n");
        exit(1);
    }

    char* fileName = argv[2];
    int port = atoi(argv[1]);

    //try openning filename
    int status = fs_init(fileName);
    if(status != 0){
        fprintf(stderr,"Error: fs_init() fails\n");
        exit(1);
    }

    //open port
    printf("Open Port: %d\n",port);
    int sd = UDP_Open(port);
    //make sure th eport is able to open
    assert(sd > -1);
    //printf("SERVER:: waiting in loop\n");

    while (1) {
        struct sockaddr_in s;
        char buffer[BUFFER_SIZE];
        char returnBuffer[BUFFER_SIZE];
        int rc = UDP_Read(sd, &s, buffer, BUFFER_SIZE);
            if (rc == 4096){
                //printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);
                int status = processConnection(buffer, returnBuffer, sd, s);
                if(status != 0){
                    //ignore current command
                    continue;
                }
	           //char reply[BUFFER_SIZE];
	           //sprintf(reply, "reply");
	           rc = UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
	   }
    }
    return 0;
}

int processConnection(char input[], char* output, int sd, struct sockaddr_in s){
    //split the buffer with the deliminator
    char* pch = strtok(input,":");
    if(pch == NULL || strlen(pch) != 1){
        //invalid input
        return -1;
    }
    int inum;
    int block;
    int type;
    int pinum;
    char* name;
    char returnBuffer[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int status;
    int rtn;
    int part;

    switch(*pch){
        case 'I':
            //printf("firstConnect\n");
            sprintf(output, "Initalized");
            break;
        case 'L':
            //printf("Lookup Called\n");
            pinum = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");
            //printf("pi:%d, name:%s\n", pinum, name);
            rtn = fs_lookup(pinum, name);
            //fs_fsync();
            sprintf(output, "%d", rtn);
            break;
        case 'C':
            //printf("Lookup Called\n");
            pinum = atoi(strtok(NULL, ":"));
            type = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");      
            //printf("pi:%d, type:%d, name:%s\n", pinum, type, name);
            rtn = fs_create(pinum, type, name);   
            fs_fsync(); 
            snprintf(output, 4096, "%d", rtn); 
            break;
        case 'R':
            //printf("Read Called\n");
            part = atoi(strtok(NULL, ":"));
            //error
            if(part != 1){
                return -1;
            }

            inum = atoi(strtok(NULL, ":"));
            block = atoi(strtok(NULL, ":"));
            char data[BUFFER_SIZE];
            rtn = fs_read(inum, data, block);
            if(rtn >= 0){
                //printf("read success\n");
                sprintf(returnBuffer, "%d", rtn);  
                status = UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
                int status = udp_wait(buffer,sd,s);
                if(status == -1){
                    return -1;
                }
                else{
                    //make sure the pong string is correct
                    char* st = strtok(buffer,":");
                    if(strncmp(st, "R", 1) != 0){
                        return -1;
                    }
                    part = atoi(strtok(NULL,":"));
                    if(part != 2){
                        return -1;
                    }

                    memcpy(output, data, 4096);
                }
                memcpy(output, data, 4096);

            }
            else{
                printf("read failed\n");
                snprintf(output, 4096, "%d", rtn);
                //fs_fsync();
            }
            //printf("inum:%d, block:%d, buffer:%s", inum, block, data);           
            break;
        case 'W':
            //printf("Write Called\n");
            inum = atoi(strtok(NULL, ":"));
            block = atoi(strtok(NULL, ":"));

            //send a reply saying we receive the first part;
            snprintf(returnBuffer,4096, "0");
            status = UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
            //wait for their reply
            status = udp_wait(buffer,sd,s);
            if(status == -1){
                return -1;
            }
            //printf("inum:%d, block:%d, buffer:%s", inum, block, buffer);
            rtn = fs_write(inum, buffer, block);
            fs_fsync();
            sprintf(output, "%d", rtn);
            break;
        case 'S':
            //printf("Stat Called\n");
            inum = atoi(strtok(NULL, ":"));
            //printf("pi:%d\n", inum);
            stat_t stat;
            fs_stat(inum, &stat);
            //fs_fsync();
            memcpy(output, &stat, 4096);
            break;
        case 'E':
            //printf("Shutdown Called\n");
            fs_close();
            //printf("fs closed\n");
            //tell the other side, the operation is completed
            snprintf(returnBuffer,4096, "0");
            UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
            //end program
            exit(0);
        case 'U':
            //printf("Unlink Called\n");
            inum = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");
            //printf("pi:%d, name:%s\n", inum, name);

            rtn = fs_unlink(inum, name);
            fs_fsync();
            sprintf(output, "%d", rtn);
            break;
        default:
            return -1;
    }

    return 0;

}


int udp_wait(char* reply, int sd, struct sockaddr_in s){
    //initalize the varaibles
    fd_set readfd;
    fd_set emptyfd;
    FD_ZERO(&emptyfd);
    FD_ZERO(&readfd);
    FD_SET(sd, &readfd);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    //char* str = " server waiting\n";
    //write(1,str,strlen(str));
    int rc = select( sd +1, &readfd, &emptyfd, &emptyfd, &timeout);
    if(rc > 0 && FD_ISSET(sd, &readfd)){
        //str = " server receive\n";
        //write(1,str,strlen(str));
        int status = UDP_Read(sd, &s, reply, BUFFER_SIZE);
        if(status != BUFFER_SIZE){
            return -1;
        }
        return 0;
    }
    //str = " server timeout\n";
    //write(1,str,strlen(str));
    return -1;
}