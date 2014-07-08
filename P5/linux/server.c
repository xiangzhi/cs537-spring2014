/* CS537 - Spring 2014 - Program 5
 * CREATED BY:  
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * Roy Fang (fang@cs.wisc.edu)
 */

#include <stdio.h>
#include <stdbool.h>
//connect with own files
#include "fs.h"
#include "udp.h"

#define BUFFER_SIZE (4096)

//prototypes
int processConnection(char input[], char* output, int sd,struct sockaddr_in s);
int udp_wait(char* reply, int sd, struct sockaddr_in s);

/**
 * main server loop
 */
int
main(int argc, char *argv[])
{

    // check program for incorrect number of arguments
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
            if (rc == BUFFER_SIZE){
                //printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);
                int status = processConnection(buffer, returnBuffer, sd, s);
                if(status != 0){
                    //ignore current command
                    continue;
                }
	           rc = UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
	   }
    }
    return 0;
}

/**
 * processConnection
 * try processing the connection
 * return 0 for success, -1 if fails
 */
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
            //for client trying to initialize the connecting with the server
            //like a heartbeat to check whether server online
            //return something to say we are.

            sprintf(output, "Initalized");
            break;

        case 'L':
            //client trying lookup

            pinum = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");
            rtn = fs_lookup(pinum, name);
            sprintf(output, "%d", rtn);
            break;

        case 'C':
            //client trying to create

            pinum = atoi(strtok(NULL, ":"));
            type = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");      
            rtn = fs_create(pinum, type, name);   
            fs_fsync(); //sync file
            snprintf(output, BUFFER_SIZE, "%d", rtn); 
            break;

        case 'R':
            //client trying to read

            part = atoi(strtok(NULL, ":"));
            //make sure its the correct format
            if(part != 1){
                return -1;
            }

            inum = atoi(strtok(NULL, ":"));
            block = atoi(strtok(NULL, ":"));
            char data[BUFFER_SIZE];
            rtn = fs_read(inum, data, block);
            if(rtn >= 0){
                //give reply
                sprintf(returnBuffer, "%d", rtn);  
                //send reply
                UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
                int status = udp_wait(buffer,sd,s);
                if(status == -1){
                    return -1;
                }
                else{
                    //make sure the second string is correct
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
                //read failed
                snprintf(output, 4096, "%d", rtn);
            }         
            break;

        case 'W':
            //client try calls write

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
            //writing the new information
            rtn = fs_write(inum, buffer, block);
            fs_fsync(); //sync the disk
            sprintf(output, "%d", rtn);
            break;

        case 'S':
            //client tries call stat

            inum = atoi(strtok(NULL, ":"));
            stat_t stat;
            fs_stat(inum, &stat);
            memcpy(output, &stat, 4096);
            break;

        case 'E':
            //client tries call shutdown
            fs_close();
            //tell the other side, the operation is completed
            snprintf(returnBuffer,4096, "0");
            UDP_Write(sd, &s, returnBuffer, BUFFER_SIZE);
            //end program
            exit(0);

        case 'U':
            //client tries call unlink

            inum = atoi(strtok(NULL, ":"));
            name = strtok(NULL, ":");
            rtn = fs_unlink(inum, name);
            fs_fsync();
            sprintf(output, "%d", rtn);
            break;
        default:
            return -1;
    }
    return 0;
}


/**
 * udp_wait
 * wait for the response from client
 * if timeout donothing except return failed
 */
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

    //wait for 5 seconds
    int rc = select( sd +1, &readfd, &emptyfd, &emptyfd, &timeout);
    //if got response
    if(rc > 0 && FD_ISSET(sd, &readfd)){
        //return the response
        int status = UDP_Read(sd, &s, reply, BUFFER_SIZE);
        if(status != BUFFER_SIZE){
            return -1;
        }
        return 0;
    }
    return -1;
}