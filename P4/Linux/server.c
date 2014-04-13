#include "cs537.h"
#include "request.h"
#include "Pthread.h"
#include <string.h>
#define TYPE_LENGTH 6
// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)> <threads >=1> <buffers >=1> <schedalg
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

//function prototype;
void* worker();

typedef struct _http_info{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE];
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];
    char filename[MAXLINE];
    char cgiargs[MAXLINE];
    rio_t rio;
    int connfd;
}http_info;

void getargs(int *port, int *thread, int *buffer, char** schedual, int argc, char *argv[])
{
    if (argc != 5) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers> <schedalg>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    if(*port < 2000){
        fprintf(stderr, "Error: port has to be more than 2000\n");
        exit(1);        
    }
    *thread = atoi(argv[2]);
    if(*thread <= 0){
        fprintf(stderr, "Error: number of thread has to be positive\n");
        exit(1);        
    }
    *buffer = atoi(argv[3]);
    if(strlen(argv[4]) > TYPE_LENGTH){
        fprintf(stderr, "Error: schedalg input is invalid\n");
        exit(1);        
    }
    strncpy(*schedual, argv[4],TYPE_LENGTH);
    //make sure strncpy copy correctly
    if(strlen(*schedual) != strlen(argv[4])){
        fprintf(stderr, "strncpy failed\n");
        exit(1);
    }
}


//global buffer stuff
int* buffer;
//size of the buffer
int size;
//use pointer;
int useptr;
//fill pointer
int fillptr;
//number of buffer full;
int numfull;
//
int request;

pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int threadNum, bufferLength;
    char* type = (char*) malloc(sizeof(char) * TYPE_LENGTH);
    struct sockaddr_in clientaddr;

    getargs(&port,&threadNum,&bufferLength, &type, argc, argv);

    buffer = (int*) malloc(sizeof(int) * bufferLength);
    pthread_t* cids = (pthread_t*) malloc(sizeof(pthread_t) * threadNum);
    size = bufferLength;
    useptr = 0;
    fillptr = 0;
    numfull = 0;
    request = 0;

    int i;
    //create those threads;
    for( i = 0; i < threadNum; i++){
        Pthread_create(&cids[i], NULL, worker, NULL);
    }

    listenfd = Open_listenfd(port);
    while (1) {
	   clientlen = sizeof(clientaddr);
	   connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        //start handling
        Pthread_mutex_lock(&mutex);
        //make sure the buffer is atleast empty
        while(numfull == size){
            Pthread_cond_wait(&cond_empty, &mutex);
        }
        //FIFO
        buffer[fillptr] = connfd;
        fillptr = (fillptr + 1) % size;
        numfull++;
        Pthread_cond_signal(&cond_full);
        Pthread_mutex_unlock(&mutex);

	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. However, for SFF, you may have to do a little work
	// here (e.g., a stat() on the filename) ...
	// 

    }
}

http_info* readHeader(int fd){
    /*
    int is_static;
   struct stat sbuf;
   char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   rio_t rio;

   Rio_readinitb(&rio, fd);
   Rio_readlineb(&rio, buf, MAXLINE);
   sscanf(buf, "%s %s %s", method, uri, version);*/
}

void* worker(){
    while(1){
        //local version of connfd;
        int connfd;
        
        //start handling
        Pthread_mutex_lock(&mutex);
        //make sure the buffer is atleast empty
        while(numfull == 0){
            Pthread_cond_wait(&cond_full, &mutex);
        }
        connfd = buffer[useptr];
        useptr = (useptr+ 1 ) % size;
        numfull--;
        request++;
        Pthread_cond_signal(&cond_empty);
        printf("request:%d\n", request);
        Pthread_mutex_unlock(&mutex);   
        //done handling
        printf("PID:%u,Handling request", pthread_self());
        //handle request
        requestHandle(connfd);
        Close(connfd);
    }
    return NULL;
}

    


 
