/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * CS537: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "cs537.h"
 #include "Pthread.h"
 #include <unistd.h>

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(1, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

void* workers(void* _filename){
  char* filename = (char*) _filename;
  //printf("file:%s\n", filename);
  int clientfd = Open_clientfd("localhost", 8559);
  clientSend(clientfd, filename);
  clientPrint(clientfd);
  Close(clientfd);  
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  int num = 3;
  pthread_t* cids = (pthread_t*) malloc(sizeof(pthread_t) * num);


  Pthread_create(&cids[0], NULL, workers, "/output.cgi?3");
  sleep(1);
  Pthread_create(&cids[2], NULL, workers, "/2.cgi?2");
  Pthread_create(&cids[3], NULL, workers, "/1.cgi?1");


  int i;
  for(i = 0; i < num; i++){
    Pthread_join(cids[i], NULL);
  }

  /* Open a single connection to the specified host and port 
  int i;
  for(i = 0; i < 10000; i++){
    clientfd = Open_clientfd(host, port);
    
    clientSend(clientfd, filename);
    clientPrint(clientfd);
      
    Close(clientfd);    
  }
  */


  exit(0);
}
