#include "cs537.h"
#include "request.h"
#include "http_info.h"
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
http_info readHeader(int fd);
void serverParseURI(http_info* info);
void copyInfo(http_info* src, http_info* dest);

int sfnfCompare(const void* p1, const void* p2);
int sffCompare(const void* p1, const void* p2);


//global buffer stuff
http_info* buffer;
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

int mode;

pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//parse the input arguments
void getargs(int *port, int *thread, int *buffer, int argc, char *argv[])
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

    if(strcmp(argv[4], "FIFO") == 0){
        mode = 0;
    }
    else if(strcmp(argv[4], "SFNF") == 0){
        mode = 1;
    }
    else if(strcmp(argv[4], "SFF") == 0){
        mode = 2;
    }
    else{
        fprintf(stderr, "Error: schedalg input is invalid\n");
        exit(1);         
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int threadNum, bufferLength;
    struct sockaddr_in clientaddr;

    getargs(&port,&threadNum,&bufferLength, argc, argv);
    //create the buffer with user specified length
    buffer = (http_info*) malloc(sizeof(http_info) * bufferLength);
    //create a list for cids
    pthread_t* cids = (pthread_t*) malloc(sizeof(pthread_t) * threadNum);
    //save the size of buffer
    size = bufferLength;
    //initalize all the  global variables;
    useptr = 0;
    fillptr = 0;
    numfull = 0;
    request = 0;
    int i;
    //create those threads;
    for( i = 0; i < threadNum; i++){
        Pthread_create(&cids[i], NULL, worker, NULL);
    }
    //open the listening port
    listenfd = Open_listenfd(port);
    while (1) {
	   clientlen = sizeof(clientaddr);
       //wait for connection
	   connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        //start handling
        Pthread_mutex_lock(&mutex);
        //make sure the buffer is atleast empty
        while(numfull == size){
            Pthread_cond_wait(&cond_empty, &mutex);
        }
        //read the http header first
        http_info info = readHeader(connfd);
        //printf("Incoming file:%s\n",info.filename);

        switch(mode){
            case 0:
                buffer[fillptr] = info;
                fillptr = (fillptr + 1) % size;
                numfull++;
                break;
            case 1:
                buffer[numfull] = info;
                numfull++;
                qsort(buffer, numfull, sizeof(http_info), sfnfCompare);
                break;
            case 2:
                buffer[numfull] = info;
                numfull++;
                qsort(buffer, numfull, sizeof(http_info), sffCompare);
                break;
        }
        Pthread_cond_signal(&cond_full);
        Pthread_mutex_unlock(&mutex);
    }
}


//parse the http_info files's pathname
//get the file name and any additional arguments with it.
void serverParseURI(http_info* info) 
{
   char *ptr;

   if (!strstr(info->uri, "cgi")) {
      // static
      strcpy(info->cgiargs, "");
      sprintf(info->filename, ".%s", info->uri);
      if (info->uri[strlen(info->uri)-1] == '/') {
         strcat(info->filename, "home.html");
      }
      info->is_static = 1;
   } else {
      // dynamic
      ptr = index(info->uri, '?');
      if (ptr) {
         strcpy(info->cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(info->cgiargs, "");
      }
      sprintf(info->filename, ".%s", info->uri);
      info->is_static = 0;
   }
}

//function to read the HTTP header from the connection descripter
http_info readHeader(int fd){
    rio_t rio;
    char buf[MAXLINE];
    http_info info;
    info.connfd = fd;
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", info.method, info.uri, info.version);
    //parse the http path
    serverParseURI(&info);
    return info;
}

//function run by the worker threads
void* worker(){
    while(1){
        //local version of http_info
        http_info info;
        
        //start handling
        Pthread_mutex_lock(&mutex);
        //make sure the buffer is atleast empty
        while(numfull == 0){
            Pthread_cond_wait(&cond_full, &mutex);
        }

        switch(mode){
            case 0:
                //copy the http_info, this is thread safe
                copyInfo(&buffer[useptr], &info);
                //copy http_info;
                useptr = (useptr+ 1 ) % size;
                numfull--;
                break;
            case 1:
                copyInfo(&buffer[0], &info);
                //move the last item to the first;
                if(numfull > 1){
                    copyInfo(&buffer[numfull-1], &buffer[0]);
                }
                //sort again
                qsort(buffer, numfull, sizeof(http_info), sfnfCompare);
                //get the correct number
                numfull--;
                break;
            case 2:
                copyInfo(&buffer[0], &info);
                /*
                int i;
                printf("QUEUE\n");
                for(i = 0; i < numfull; i++){
                    printf("file:%s\n", buffer[i].filename);
                }
                */
                //move the last item to the first;
                if(numfull > 1){
                    copyInfo(&buffer[numfull-1], &buffer[0]);
                }
                //sort again
                qsort(buffer, numfull, sizeof(http_info), sffCompare);
               //get the correct number
                numfull--;
                break;
        }
        request++;

        Pthread_cond_signal(&cond_empty);
        //printf("PID:%u, request:%d,Handling request\n", (unsigned int)pthread_self(), request);
        //printf("path:%s\n", info.uri);
        //printf("filename:%s\n", info.filename);
        //printf("isStatic:%d\n", info.is_static);
        Pthread_mutex_unlock(&mutex);   
        //done handling
        //handle request
        requestHandle(&info);
        //close and free resources;
        Close(info.connfd);
    }
    return NULL;
}


//a thread safe copyer to copy http_info
void copyInfo(http_info* src, http_info* dest){
    dest->is_static = src->is_static;
    strncpy(dest->method, src->method, MAXLINE);
    strncpy(dest->uri, src->uri, MAXLINE);
    strncpy(dest->version, src->version, MAXLINE);
    strncpy(dest->filename, src->filename, MAXLINE);
    strncpy(dest->cgiargs, src->cgiargs, MAXLINE);
    dest->connfd = src->connfd;
}
    
//qsort function when it is the shortest file name first
int sfnfCompare(const void* p1, const void* p2){
    int num = strlen(((http_info*)p1)->filename) - strlen(((http_info*)p2)->filename);
    return num;
}

//qsort function for smallest file first
int sffCompare(const void* p1, const void* p2){
    struct stat file1;
    struct stat file2;
    Stat(((http_info*)p1)->filename, &file1);
    //printf("file %llu size:%llu\n",file1.st_ino,  file1.st_size);
    //printf("file %llu size:%llu\n", file2.st_ino, file2.st_size);
    Stat(((http_info*)p2)->filename, &file2);
    return file1.st_size - file2.st_size;
}

 
